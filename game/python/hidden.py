def hidden():
	return 'inaccessible!'

# Try to bless ourselves real good.
import pennmush
import sys

try:
	pennmush.bless(sys.modules[__name__])
except ValueError, ex:
	pass
