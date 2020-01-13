import sys, re

block = re.compile(r'[^{]*({[^\}]*\})\s*')
apifunc = re.compile('([^\(]*)\( *\*([^\)]*)\) ?\(([^\)]*)\)')
ad = {'void':'v','handle':'i','dword':'u','int':'i', 'long':'i', 'uint':'u', 'byte':'c', 'char': 'c', 'size_t': 'u', 'security_information': 'u', 'word': 'w', 'atom':'w', 'short': 'w', 'double': 'd', 'time_t': 'u', 'uint64_t':'I', 'long64':'I', 'vec3_t':'p', 'qboolean':'i', 'float':'f', '...' : 'V', 'hsprite': 'i', 'tricullstyle':'u'}

def parsearg(arg):
	arg = arg.replace('unsigned int', 'uint')
	arg = arg.replace('unsigned short', 'word')
	arg = arg.replace('unsigned long', 'uint')
	arg = arg.replace('unsigned char', 'byte')
	arg = arg.replace('const ', '')

	l = arg.split(' ')[0].lower()

	# pointers
	if '*' in arg:
		return 'p'

	# functions (need process callbacks)
	if l.startswith('pfn') or l.endswith('_func_t'):
		return '[p]' # callback?

	if l == 'wrect_t':
		return 'uuuu'

	# enums
	if l.endswith('_t') or l.endswith('_type'):
		return 'u'

	if l in ad:
		return ad[l]
	return '('+arg+')'

def load(n):
	f = open(n)
	# strip comments and macros
	lines = [ l.split('//')[0].split('#')[0] for l in f]
	tokens = [t.replace('\t',' ').replace('  ',' ').replace('  ',' ').replace('  ',' ').strip() for t in ''.join(lines).replace('\n',' ').split(';')]
	return ';'.join(tokens)

f = load(sys.argv[1])
blocks = block.findall(f)
funcs = blocks[int(sys.argv[2])][1:-2].split(';')
for func in funcs:
	m = apifunc.match(func)
	if not m:
		print('not matched: ', func.split(' '))
	else:
		g =  m.groups()
		print( sys.argv[3] + g[1].strip() + sys.argv[4] + parsearg(g[0].strip())+'F'+ (''.join([parsearg(gg.strip()) for gg in g[2].split(',')])) + sys.argv[5])