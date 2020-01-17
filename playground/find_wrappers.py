import sys
import re
r = re.compile(r'FF\([^)]*\)')
values = ['E', 'e', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'V', "O", "S"]
deltas = [0, 0, 4, 4, 4, 4, 8, 4, 4, 4, 8, 4, 8, 12, 12, 4, 4, 4, 0, 4, 4]
def d(v):
	return deltas[values.index(v)]
r2 = re.compile(r'[ ]*{ ?"([^,]*) ?" ?, ?([^,]*), ?([^,]*),[ ]?([^}]*)},?')
sys.stdout.write('#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))\n#error meh!\n#endif\n')

for arg in sys.argv[1:]:
	for line in open(arg):
		if 'FF(' in line and not 'GLFF' in line and not '#' in line and not 'UNDEFF' in line:
			args = ')'.join('('.join(line.split('(')[1:]).split(')')[0:-1]).split(',')
#			print line, args[-2]
			fname = args[0].strip()
			signature = args[-2].strip()
			if not 'F' in signature:
				#print args,line
				continue
			sys.stdout.write('GO('+fname+','+signature+')\n')
		if 'WRAP' in line and not '#' in line:
			args = line.split('(')[1].split(')')[0].split(',')
			fname = args[0].strip()
			signature = args[1].strip()
			if not 'F' in signature:
				continue
			sys.stdout.write('GO('+fname+','+signature+')\n')
			