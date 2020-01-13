import sys
s = open(sys.argv[1]).read()

#s = s.replace('\n','');
spl = s.split('\n')
ad = {'void':'v','handle':'i','dword':'u','int':'i', 'long':'i', 'uint':'u', 'byte':'c', 'char': 'c', 'size_t': 'u', 'security_information': 'u', 'word': 'w', 'atom':'w', 'short': 'w', 'double': 'd', 'time_t': 'u', 'uint64_t':'I', 'long64':'I'}
def func(arg):
	arg=arg.lower().replace('\t','').replace('const','')
	arg = arg.split(' ')[0]
	if arg.startswith('h'):
		return 'i'
	if 'bool' in arg or 'dword' in arg or 'lcid' in arg or 'lctype' in arg:
		return 'u'
	if arg.startswith('lp') or arg.startswith('p') or 'ptr' in arg or '*' in arg or 'proc' in arg or 'func' in arg:
		return 'p'
	if arg in ad:
		return ad[arg]
	if arg == '':
		return 'v'
	return '('+arg+')'

for p in spl:
	try:
		if '#' in p or '/' in p:
			p = ''.join(p.split('\n')[2:])
#			print(p)
		p = p.replace('\n','')
#		if not 'WINAPI' in p: continue
		ret = p.split(' ')[0]
		left = p.split('exp')[1]
		spl2 = left.split('(')
		name = spl2[0]
		args = spl2[1].split(')')[0].split(',')
		print(name,func(ret)+'F'+''.join([func(a) for a in args]).replace('vv','v').replace('vv','v'))
	except Exception:pass