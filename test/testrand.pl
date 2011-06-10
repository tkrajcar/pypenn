run tests:
test('rand.1', $god, 'think rand(-1)', '#-1');
test('rand.2', $god, 'think rand(0)', '#-1');
test('rand.3', $god, 'think rand(1)', '0');
test('rand.4', $god, 'think rand(10)', '^\d\s*$');
test('rand.5', $god, 'think rand(0,0)', '0');
test('rand.6', $god, 'think rand(1,1)', '1');
test('rand.7', $god, 'think rand(2,1)', '#-1');
test('rand.8', $god, 'think rand(0,9)', '^\d\s*$');

