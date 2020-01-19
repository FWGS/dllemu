import sys
import re


def process(func, ifile):
	r = re.compile(r'[A-z \*]* \*?'+func+' ?\([^)]*\) ?$')
	f = open(ifile)
	lines = f.readlines()
	matches = 0
	f.close()
	for i in range(len(lines)):
		line = lines[i]
		if ';' in line:
			continue
		m  = r.match(line)
		if not m:
			continue
		if not '{' in lines[i+1]:
			continue
		matches += 1
		if 'GAME_EXPORT' in line:
			continue
		#print(m)
		#print(m.groups())
		#print(dir(m))
		lines[i] = line.replace(' '+func, ' GAME_EXPORT ' + func)
		print(line)
	if matches == 1:
		f = open(ifile,'w')
		f.writelines(lines)
		f.close()
	return matches

def main(func, files):
	matches = 0
	for arg in files:
		fmatches = process(sys.argv[1], arg)
		if fmatches > 1:
			print(arg + ': MATCHES FOR ' + func + ' ' + str(matches))
		matches += fmatches
	if matches == 0:
		print('NO MATCHES FOR '+func)

main(sys.argv[1], sys.argv[2:])
