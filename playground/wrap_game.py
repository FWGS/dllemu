import sys, re

block = re.compile(r'[^{]*({[^\}]*\})\s*')
apifunc = re.compile('([^\(]*)\( *\*([^\)]*)\) ?\(([^\)]*)\)')
ad = { 'void *': 'p','void':'v','long long': 'I', 'uint': 'u', 'handle':'i','dword':'u','int':'i', 'long':'i', 'uint':'u', 'byte':'c', 'char': 'c', 'size_t': 'u', 'security_information': 'u', 'word': 'w', 'atom':'w', 'short': 'w', 'double': 'd', 'time_t': 'u', 'uint64_t':'I', 'long64':'I', 'vec3_t':'p', 'qboolean':'i', 'float':'f', '...' : 'V', 'hsprite': 'i', 'tricullstyle':'u'}

values = ['E', 'e', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'V', "O", "S"]
types = ["x86emu_t*", "x86emu_t**", "void", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "float", "double", "long double", "double", "intptr_t", "uintptr_t", "void*", "void*", "int32_t", "void*"]
deltas = [0, 0, 4, 4, 4, 4, 8, 4, 4, 4, 8, 4, 8, 12, 12, 4, 4, 4, 0, 4, 4]
def t(v):
	return types[values.index(v)]
def d(v):
	return deltas[values.index(v)]

def v(a):
	return t(parsearg(a))


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
		#print( sys.argv[3] + g[1].strip() + sys.argv[4] + parsearg(g[0].strip())+'F'+ (''.join([parsearg(gg.strip()) for gg in g[2].split(',')])) + sys.argv[5])
		args = g[2].split(',')
		argnames = []
		argpushes = []
		i = 0
		delta = 0
		funcname = g[1].strip()
		retp = parsearg(g[0].strip())
		rettype = t(retp)
		for a in g[2].split(','):
			a = parsearg(a.strip())
			if a == 'v':
				argnames.append('void')
				break
			argnames.append(t(a)+' '+'a'+str(i))
			dl = d(a)
			argpushes.append('memcpy(R_ESP - ' + str(delta+dl)+', &a'+str(len(args)-i-1)+','+str(dl)+')')
			i+=1
			delta+=dl
		retfv = ['f', 'd']
		retlv = ['I', 'U']
		if retp == 'v':
			ret = ''
		elif retp in retfv:
			ret = 'do_fpu_pop(emu);return ST0val;'
		elif retp in retlv:
			ret = 'return ((uint64_t)R_EAX)|(((uint64_t)R_EDX)<<32);'
		else:
			 ret = 'return *(('+rettype+'*)&R_EAX);'
		print(rettype+' wrap_'+funcname+'( '+', '.join(argnames)+' ){\n'+ ';\n'.join(argpushes)+';\nR_ESP -= '+str(delta)+';\nDynaCall( emu, api->'+funcname+' );\nR_ESP += '+str(delta)+';\n'+ret+'\n}')
#		print(v(g[0].strip()),[v(gg.strip())+' '+'0' for gg in g[2].split(',')])