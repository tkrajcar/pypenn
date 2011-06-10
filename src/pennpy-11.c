#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <assert.h>

/* PennMUSH headers. */
#include "config.h"
#include "conf.h"
#include "externs.h"
#include "flags.h"
#include "parse.h"
#include "function.h"
#include "log.h"

#include "pennpy.h"

/*
 * Configuration constants.
 */
#define MAIN_PY "python/main.py"

/*
 * Forward declarations.
 */
static PyObject *cu5_pennpy_resolve(char *dotted_name);
static int cu5_pennpy_bless_module(PyObject *module_obj);
static int cu5_pennpy_set_timer(PyObject *hook_obj);

/*
 * Python to PennMUSH.
 */

#define PennPy_METHOD(name) \
	static PyObject * \
	name(PyObject *self __attribute__ ((__unused__)), PyObject *args)

/* Reference to __main__.  Only usable when the interpreter is initialized. */
static PyObject *cu5_pennpy_main = NULL;
static PyObject *cu5_pennpy_main_dict = NULL;

PennPy_METHOD(cu5_pennpy_meth_bless)
{
	static int frozen = 0;

	PyObject *module_obj;

	if (!PyArg_ParseTuple(args, "O", &module_obj)) {
		/* Threw an exception. */
		return NULL;
	}

	if (module_obj == Py_None) {
		/* Freeze blessing state. */
		frozen = 1;
		Py_RETURN_NONE;
	}

	if (!PyModule_Check(module_obj)) {
		/* Not a module, raise a TypeError. */
		PyErr_SetString(PyExc_TypeError, "Can only bless modules");
		return NULL;
	}

	if (frozen) {
		/* Only accept None after freezing. */
		PyErr_SetString(PyExc_ValueError, "Blessings frozen");
		return NULL;
	}

	if (!cu5_pennpy_bless_module(module_obj)) {
		/* Internal error, propagate exception. */
		return NULL;
	}

	Py_RETURN_NONE;
}

PennPy_METHOD(cu5_pennpy_meth_set_timer)
{
	PyObject *hook_obj;

	if (!PyArg_ParseTuple(args, "O", &hook_obj)) {
		/* Threw an exception. */
		return NULL;
	}

	if (hook_obj == Py_None) {
		/* Clear the timer hook. */
		hook_obj = NULL;
	} else if (!PyCallable_Check(hook_obj)) {
		/* Not callable, raise a TypeError. */
		PyErr_SetString(PyExc_TypeError, "Not callable");
		return NULL;
	}

	if (!cu5_pennpy_set_timer(hook_obj)) {
		/* Internal error, propagate exception. */
		return NULL;
	}

	Py_RETURN_NONE;
}

PennPy_METHOD(cu5_pennpy_meth_notify)
{
	PyObject *targets_obj, *targets_iter, *next_obj;
	const char *message;

	/* Get arguments. */
	if (!PyArg_ParseTuple(args, "Os", &targets_obj, &message)) {
		/* Threw an exception. */
		return NULL;
	}

	/* Iterate over targets. */
	if (!(targets_iter = PyObject_GetIter(targets_obj))) {
		/* Threw an exception. */
		return NULL;
	}

	while ((next_obj = PyIter_Next(targets_iter))) {
		/* Notify target. */
		dbref target = PyInt_AsLong(next_obj);
		Py_DECREF(next_obj);

		if (target == -1 && PyErr_Occurred()) {
			/* Threw an exception during iteration. */
			break;
		}

		if (GoodObject(target) && !IsGarbage(target)) {
			notify(target, message);
		}
	}

	Py_DECREF(targets_iter);

	/* Done. */
	if (PyErr_Occurred()) {
		/* Threw an exception during iteration. */
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyMethodDef cu5_pennpy_module[] = {
	{
		"bless", cu5_pennpy_meth_bless, METH_VARARGS,
		"bless(module)\n"
		"Make module accessible to PyCall(). bless(None) prevents further blessing."
	},

	{
		"set_timer", cu5_pennpy_meth_set_timer, METH_VARARGS,
		"set_timer(callable)\n"
		"Set timer hook to the passed callable. set_timer(None) clears the timer hook."
	},

	{
		"notify", cu5_pennpy_meth_notify, METH_VARARGS,
		"notify(target, message)\n"
		"Notify the iterable, target, of message."
	},

	{ NULL, NULL, 0, NULL }
};

/*
 * PennMUSH to Python.
 */

static int cu5_pennpy_enabled = 0;

static void
cu5_pennpy_exception(char *buff, char **bp)
{
	const char *ex_name;
	PyObject *ex_type;

	/*
	 * Report exception.  We don't return specifics; if you need them,
	 * handle that in the Python code and return a string.
	 */
	ex_name = "PYTHON"; /* catch-all exception "name" */

	ex_type = PyErr_Occurred();
	if (PyExceptionClass_Check(ex_type)) {
		/* Exception type is a valid exception class. */
		ex_name = PyExceptionClass_Name(ex_type);
		if (ex_name) {
			/* Exception class has a name. */
			char *ex_short_name = strrchr(ex_name, '.');
			if (ex_short_name) {
				/* blah.blah.ExceptionName */
				ex_name = ex_short_name + 1;
			}
		} else {
			/* Shouldn't happen, but extra cautious. */
		}
	} else {
		/*
		 * This must be an old-style string exception, but those are
		 * deprecated and now too rare for us to bother identifying.
		 */
	}

	safe_format(buff, bp, T("#-1 %s EXCEPTION"), ex_name);

	/* Dump the stack to stderr and clear the error indicator. */
	do_rawlog(LT_ERR, "PennPy: Python exception:");
	PyErr_Print();
}

static PyObject *
cu5_pennpy_args(int nargs, char *args[], int arglens[])
{
	PyObject *args_obj;
	int ii;

	if (!(args_obj = PyTuple_New(nargs))) {
		/* Out of memory. */
		PyErr_Clear();
		return NULL;
	}

	for (ii = 0; ii < nargs; ii++) {
		PyObject *str_obj;

		str_obj = PyString_FromStringAndSize(args[ii], arglens[ii]);
		if (!str_obj) {
			/*
			 * Out of memory.  We need to finish initializing the
			 * tuple before we free it, so we set remaining tuple
			 * fields to Py_INCREF(Py_None).
			 *
			 * This should be pretty rare, so speed is unimportant.
			 */
			for (; ii < nargs; ii++) {
				Py_INCREF(Py_None);
				PyTuple_SET_ITEM(args_obj, ii, Py_None);
			}

			/* Free tuple.  TODO: Doesn't throw exceptions, yes? */
			Py_DECREF(args_obj);
			return NULL;
		}

		PyTuple_SET_ITEM(args_obj, ii, str_obj);
	}

	return args_obj;
}

static int
cu5_pennpy_unparse(PyObject *value, char *buff, char **bp)
{
	char *value_str;

	if (value == Py_None) {
		/* No value. */
		return 0;
	}

	/* Coerce value to string. */
	if (!(value = PyObject_Str(value))) {
		/* Swallow exception. */
		PyErr_Clear();
		return 0;
	}

	/* Get string. */
	if (!(value_str = PyString_AsString(value))) {
		/* Swallow exception. */
		PyErr_Clear();
		Py_DECREF(value);
		return 0;
	}

	safe_str(value_str, buff, bp);
	Py_DECREF(value);
	return 1;
}

FUNCTION(cu5_pennpy_fun_pycall)
{
	PyObject *result, *call_obj, *args_obj;

	assert(nargs > 0);

	/* Check for PyCall permission. */
	if (!has_power_by_name(executor, "PyCall", NOTYPE)) {
		safe_str(T(e_perm), buff, bp);
		return;
	}

	if (!cu5_pennpy_enabled) {
		/* PennPy disabled because of initialize failure. */
		safe_str(T(e_disabled), buff, bp);
		return;
	}

	/* Find function. */
	if (!(call_obj = cu5_pennpy_resolve(args[0]))) {
		PyErr_Clear();
		safe_str(T(e_match), buff, bp);
		return;
	}

	if (!PyCallable_Check(call_obj)) {
		Py_DECREF(call_obj);
		safe_str(T("#-1 NOT CALLABLE"), buff, bp);
		return;
	}

	/* Collect arguments. */
	if (!(args_obj = cu5_pennpy_args(nargs - 1, args + 1, arglens + 1))) {
		/* Shouldn't happen normally.  Maybe just panic. */
		Py_DECREF(call_obj);
		safe_str(T("#-1 INTERNAL ERROR"), buff, bp);
		return;
	}

	/* Call function with arguments. */
	result = PyObject_CallObject(call_obj, args_obj);
	Py_DECREF(args_obj);
	Py_DECREF(call_obj);

	if (!result) {
		/* Call threw exception. */
		cu5_pennpy_exception(buff, bp);
		return;
	}

	/* Return result as a string. */
	if (!cu5_pennpy_unparse(result, buff, bp)) {
		safe_str(T("#-1 RETURN TYPE ERROR"), buff, bp);
	}

	Py_DECREF(result);
}

FUNCTION(cu5_pennpy_fun_pyeval)
{
	PyObject *result;

	assert(nargs == 1);

	/* Check for PyEval permission. */
	if (!has_power_by_name(executor, "PyEval", NOTYPE)) {
		safe_str(T(e_perm), buff, bp);
		return;
	}

	if (!cu5_pennpy_enabled) {
		/* PennPy disabled because of initialize failure. */
		safe_str(T(e_disabled), buff, bp);
		return;
	}

	/* Evalute string. */
	result = PyRun_String(args[0], Py_eval_input,
	                      cu5_pennpy_main_dict, cu5_pennpy_main_dict);
	if (!result) {
		/* Evaluation threw exception. */
		cu5_pennpy_exception(buff, bp);
		return;
	}

	/* Return result as a string. */
	if (!cu5_pennpy_unparse(result, buff, bp)) {
		safe_str(T("#-1 RETURN TYPE ERROR"), buff, bp);
	}

	Py_DECREF(result);
}

FUNCTION(cu5_pennpy_fun_pyrun)
{
	PyObject *result;

	assert(nargs == 1);

	/* Check for PyEval permission. */
	if (!has_power_by_name(executor, "PyEval", NOTYPE)) {
		safe_str(T(e_perm), buff, bp);
		return;
	}

	if (!cu5_pennpy_enabled) {
		/* PennPy disabled because of initialize failure. */
		safe_str(T(e_disabled), buff, bp);
		return;
	}

	/* Evalute string. */
	result = PyRun_String(args[0], Py_file_input,
	                      cu5_pennpy_main_dict, cu5_pennpy_main_dict);
	if (!result) {
		/* Evaluation threw exception. */
		cu5_pennpy_exception(buff, bp);
		return;
	}

	/* Ignore result. */
	Py_DECREF(result);
}

void
cu5_pennpy_functions(void)
{
	function_add("PYCALL", cu5_pennpy_fun_pycall, 1, INT_MAX, FN_REG);
	function_add("PYEVAL", cu5_pennpy_fun_pyeval, 1, 1, FN_REG);
	function_add("PYRUN", cu5_pennpy_fun_pyrun, 1, 1, FN_REG);
}

/*
 * Timer stuff.
 */

static PyObject *cu5_pennpy_timer_hook = NULL;

static int
cu5_pennpy_set_timer(PyObject *hook_obj)
{
	/* Remove old hook. */
	Py_CLEAR(cu5_pennpy_timer_hook);

	/* Set new hook. */
	if (hook_obj) {
		Py_INCREF(hook_obj);
		cu5_pennpy_timer_hook = hook_obj;
	}

	return 1;
}

void
cu5_pennpy_timer(void)
{
	PyObject *result;

	if (!cu5_pennpy_enabled || !cu5_pennpy_timer_hook) {
		/* No timer hook available. */
		return;
	}

	/* Call timer hook. */
	result = PyObject_CallObject(cu5_pennpy_timer_hook, NULL);
	if (!result) {
		do_rawlog(LT_ERR, "PennPy: Timer hook threw exception");
		PyErr_Print();

		/* Disable timer hook.  Annoying, so don't throw exceptions. */
		cu5_pennpy_set_timer(NULL);
		return;
	}

	/* Discard result. */
	Py_DECREF(result);
}

/*
 * Name resolution.
 */

typedef struct CU5_PennPy_Module CU5_PennPy_Module;
struct CU5_PennPy_Module {
	CU5_PennPy_Module *next;

	PyObject *module_obj;
};

static CU5_PennPy_Module *cu5_pennpy_blessed = NULL;
static CU5_PennPy_Module **cu5_pennpy_blessed_last = &cu5_pennpy_blessed;

/* Note that dotted_name will remain the same, but must be modifiable. */
static PyObject *
cu5_pennpy_resolve(char *dotted_name)
{
	CU5_PennPy_Module *blessed;

	PyObject *parent_obj;
	PyObject *resolved_obj;

	Py_INCREF(cu5_pennpy_main);
	parent_obj = cu5_pennpy_main;

	for (;;) {
		char old_ch, *cp;

		/* Truncate name component at . or NUL. */
		for (cp = dotted_name; *cp != '\0' && *cp != '.'; cp++)
			;

		/* Resolve name. */
		old_ch = *cp;
		*cp = '\0';
		resolved_obj = PyObject_GetAttrString(parent_obj, dotted_name);
		*cp = old_ch;

		Py_DECREF(parent_obj); /* only need pointer for identity */

		if (!resolved_obj) {
			/* Couldn't find it, propagate exception. */
			return NULL;
		}

		/* Recurse to new object. */
		if (old_ch == '\0') {
			/* End of name. */
			break;
		}

		dotted_name = cp + 1;
		parent_obj = resolved_obj;
	}

	/*
	 * Check blessing of parent_obj.  We can speed this by marking the
	 * blessing on the module (but allows blessing to occur by accident),
	 * or using a radix hash on the pointer.
	 */
	for (blessed = cu5_pennpy_blessed; blessed; blessed = blessed->next) {
		if (blessed->module_obj == parent_obj) {
			/* Go with my blessing. */
			return resolved_obj;
		}
	}

	Py_DECREF(resolved_obj);
	PyErr_SetString(PyExc_TypeError, "Trying to access unblessed object");
	return NULL;
}

static int
cu5_pennpy_bless_module(PyObject *module_obj)
{
	CU5_PennPy_Module *new_blessed;

	/* Create new blessing item. */
	new_blessed = (CU5_PennPy_Module *)malloc(sizeof(CU5_PennPy_Module));
	if (!new_blessed) {
		PyErr_NoMemory();
		return 0;
	}

	new_blessed->next = NULL;

	Py_INCREF(module_obj);
	new_blessed->module_obj = module_obj;

	/*
	 * Add blessing to the end of the list.  This implies that you'll want
	 * to put more important modules first, so they get checked quicker.
	 */
	*cu5_pennpy_blessed_last = new_blessed;
	cu5_pennpy_blessed_last = &new_blessed->next;

	return 1;
}

/*
 * Startup/shutdown stuff.
 */

void
cu5_pennpy_initialize(void)
{
	FILE *main_py_fp;

	assert(cu5_pennpy_main == NULL);

	do_rawlog(LT_ERR, T("PennPy: Initializing"));

	/* Initialize Python without installing signal handlers. */
	Py_InitializeEx(0);

	/* Borrow a reference to __main__. */
	if (!(cu5_pennpy_main = PyImport_AddModule("__main__"))) {
		/* This shouldn't happen, but you never know. */
		mush_panic(T("PennPy: Can't get reference to __main__"));
	}

	cu5_pennpy_main_dict = PyModule_GetDict(cu5_pennpy_main);

	/* Export hooks as __pennmush__ module. */
	if (!Py_InitModule("__pennmush__", cu5_pennpy_module)) {
		/* Can't initialize __pennmush__ module. */
		mush_panic(T("PennPy: Can't initialize __pennmush__ module"));
	}

	/* Execute game/python/main.py. */
	if (!(main_py_fp = fopen(MAIN_PY, "r"))) {
		/* Can't run main.py. */
		do_rawlog(LT_ERR, T("PennPy: Can't open %s"), MAIN_PY);
		return;
	}

	if (PyRun_SimpleFile(main_py_fp, MAIN_PY) != 0) {
		/* Something wrong with main.py. */
		do_rawlog(LT_ERR, T("PennPy: Failed to execute %s"), MAIN_PY);
		fclose(main_py_fp);
		return;
	}

	fclose(main_py_fp);

	/* Ready. */
	cu5_pennpy_enabled = 1;
}

void
cu5_pennpy_finalize(void)
{
	assert(cu5_pennpy_main != NULL);

	cu5_pennpy_enabled = 0;

	do_rawlog(LT_ERR, T("PennPy: Finalizing"));

	/* Clean up (some) Python interpreter resources.  Not exhaustive. */
	Py_Finalize();
}

void
cu5_pennpy_flags(FLAGSPACE *flags __attribute__ ((__unused__)))
{
	if (strcmp(flags->name, "POWER") == 0) {
		/*
		 * Granting the PyEval power is highly dangerous.  Note that
		 * the full capabilities of the Python environment will be
		 * available to any object with PyEval powers.
		 */
		add_power("PyEval", '\0', NOTYPE, F_GOD | F_LOG, F_GOD);

		/*
		 * Granting the PyCall power is less dangerous.  It's slightly
		 * more dangerous than hard code, though, because you can
		 * inject dangerous code at runtime via PyEval(), then execute
		 * it using just PyCall().
		 */
		add_power("PyCall", '\0', NOTYPE, F_WIZARD | F_LOG, F_WIZARD);
	}
}
