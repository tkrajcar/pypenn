# Entry point from PennPy.

#
# Fiddle with our runtime environment.
#
import sys

# PennMUSH closes stdin and stdout, so redirect to stderr for safety.
sys.stdin = sys.stderr
sys.stdout = sys.stderr

# Include GAMEDIR/python in our import path.
sys.path.insert(0, 'python')

del sys

# Bless __main__, then prevent further blessing.
import pennmush

import __main__
pennmush.bless(__main__)

pennmush.bless(None)

#
# Some test code for now.
#
def smoosh_string(*args):
	return ' '.join(args)

def test_fun(arg):
	return 42

def empty_fun():
	pass

def test_notify(target, message):
	# FIXME: This isn't how you detect dbrefs for real.
	int_list = [int(pennmush.dbref(dbref)) for dbref in target.split()]
	pennmush.notify(int_list, message)

def test_call_info():
	ctx = pennmush.CallContext()
	return '{0} {1} {2}'.format(ctx.executor, ctx.caller, ctx.enactor)

import hidden

#
# Test timer code.
#
import time

def periodic():
	print 'periodic() says: The current time is', time.asctime()
	
	# Test disable after exception.
	raise AssertionError("I've been bad")

pennmush.set_timer(periodic)

import __pennmush__
print '_DB_KEY_NAME =', __pennmush__._DB_KEY_NAME


#
# Reloader. TODO this is utter craps.
#
import sys
def reload(module):
    __builtins__.reload(sys.modules[module])

#
# General utilities.
#
def colortable():
    from ll import ansistyle
    (executor, caller, enactor) = pennmush.call_info()
    retval = ""
    for i in range(0, 264):
        t = ansistyle.Text(i, str(i))
        retval = retval + " " + t.string()
    pennmush.notify([enactor], retval)
    retval = ""
    for i in range(265, 511):
        t = ansistyle.Text(i, str(i))
        retval = retval + " " + t.string()
    pennmush.notify([enactor], retval)

#
# Honor code.
#
import honor
def honor_pts(victim):
    return honor.pts(victim)

def honor_log_view(victim):
    honor.log_view(victim)

def honor_top_list(clan="none"):
    honor.top_list(clan)
