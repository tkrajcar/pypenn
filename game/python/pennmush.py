import weakref

# Import everything from the internal __pennmush__ module.
from __pennmush__ import *

# DBref.
class dbref(object):
	__slots__ = ('__weakref__', '__int', '__str')
	__interned = weakref.WeakValueDictionary()
	
	def __new__(cls, value):
		if isinstance(value, basestring):
			if value[0] is not '#':
				raise ValueError('Not a dbref string')
			value = value[1:]
		
		int_value = int(value)
		obj = cls.__interned.get(int_value, None)
		if obj is None:
			obj = object.__new__(cls)
			cls.__interned[int_value] = obj
		
		obj.__int = int_value
		obj.__str = '#' + str(int_value)
		return obj
	
	def __int__(self):
		return self.__int
	
	def __str__(self):
		return self.__str
	
	def __nonzero__(self):
		return self.__int >= 0

# Call context.
class CallContext(object):
	__slots__ = ('executor', 'caller', 'enactor')
	
	def __init__(self):
		ctx = call_info()
		if not ctx:
			raise ValueError('Not in a call context')
		
		(self.executor, self.caller, self.enactor) = map(dbref, ctx[:3])

# Pythonic interface to PennMUSH soft code API.
class _PennSoftAPI(object):
	__slots__ = ()
	
	def __getattr__(self, name):
		return lambda *args: penn_call(name, *args)

api = _PennSoftAPI()

# Pythonic interface to PennMUSH objects.  Be careful about how we define the
# proxy classes, since the underlying DB object can be deleted and recreated
# whenever PennMUSH has control.  It's only really good transiently (within a
# single PyCall into the Python interpreter).
class _PennDBProxy(object):
	__slots__ = ()
	
	def __getitem__(self, dbref):
		if not valid_dbref(int(dbref)):
			raise KeyError('No such dbref')
		return _PennObjectProxy(dbref)

class _PennObjectProxy(object):
	__slots__ = ('dbref', 'attrs')
	
	def __init__(self, dbref):
		self.dbref = dbref
		self.attrs = _PennAttributeProxy(dbref)

class _PennAttributeProxy(object):
	__slots__ = ('__dbref')
	
	def __init__(self, dbref):
		self.__dbref = int(dbref)
	
	def __getitem__(self, name):
		return get_attr(self.__dbref, name)
	
	def __setitem__(self, name, value):
		if value is not None:
			value = str(value)
		set_attr(self.__dbref, name, value)
	
	def __delitem__(self, name):
		set_attr(self.__dbref, name, None)

db = _PennDBProxy()
