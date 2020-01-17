import sys
import re
r = re.compile(r'FF\([^)]*\)')
values = ['E', 'e', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'V', "O", "S"]
deltas = [0, 0, 0, 4, 4, 4, 8, 4, 4, 4, 8, 4, 8, 12, 12, 4, 4, 4, 0, 4, 4]
def d(v):
	return deltas[values.index(v)]
r2 = re.compile(r'[ ]*{ ?"([^,]*) ?" ?, ?([^,]*), ?([^,]*),[ ]?([^}]*)},?')

                        
for line in open(sys.argv[1]):
	if 'FF(' in line and not 'GLFF' in line and not '#' in line and not 'UNDEFF' in line:
		try:
			args = line.split('(')[1].split(')')[0].split(',')
			fname = args[0].strip()
			if len(args) == 3:
				ordinal = args[1].strip()
				signature = list(args[2].strip())
			else:
				signature = list(args[1].strip())
				ordinal = '-1'
			asize = 0
			if signature[1] == 'S':
				signature[1] = 'F'
				for v in signature[2:]:
					asize += d(v)
#				print fname
			line = r.sub('FF('+ fname +', '+ordinal+', '+ ''.join(signature) +', '+str(asize)+')',line)
		except Exception as e:
			print(e)
		#sys.stdout.write(line)
	m = r2.match(line)
	if m and not '#' in line:
		g = m.groups()
		line = '    WR2FF("'+g[0].strip()+'", '+g[1].strip()+', '+g[2].strip()+', '+g[3].strip()+', 0)\n'
		#print line
	sys.stdout.write(line)