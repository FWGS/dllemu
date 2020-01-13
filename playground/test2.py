import sys

li = dict([l.split(' ') if len(l.split(' ')) == 2 else ['',''] for l in open(sys.argv[2]).read().split('\n')])

for line in open(sys.argv[1]):
	if 'FF(' in line and not 'GLFF' in line:
		fname = line.split('(')[1].split(',')[0].split(')')[0]
#		print fname
		if fname in li:
			line = line.replace(')',','+li[fname]+')')
	sys.stdout.write(line)