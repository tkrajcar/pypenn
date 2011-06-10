# Stub moddule for testing PennPy code without PennMUSH.

# Emulate DB keys.
_DB_KEY_NAME = 0

# Emulate call_info.
def call_info():
	return None

# Emulate notify.
def notify(target, message):
	for dbref in target:
		print dbref, '=>', message

# Emulate blessing.
_fake_blessable = True

def bless(mod_obj):
	global _fake_blessable
	
	if not _fake_blessable:
		raise ValueError
	
	if mod_obj is None:
		_fake_blessable = False

# Emulate DB access.
_fake_db = {0: ('One', {'LAST': 42})}

def valid_dbref(dbref):
	return (dbref in _fake_db)

def get_attr(dbref, attr):
	fake_attrs = _fake_db[dbref][1]
	return fake_attrs.get(attr, None)

def set_attr(dbref, attr, value):
	fake_attrs = _fake_db[dbref][1]
	if value is None:
		del _fake_db[dbref][1][attr]
	else:
		_fake_db[dbref][1][attr] = value

# Timer hook stub.
def set_timer(hook):
	pass

# Emulate soft code API access.
def penn_call(name, *args):
	return '#-1'
