f = open('Assembly.asm', 'r+')
g = open('finalAssembly.asm', 'w')
dict = {}
for lines in f:
	line = lines.split(' ')
	for i in line:
		i = i.strip()
		if '_' in i:
			if i not in dict:
				dict[i] = str(3000 + 2*len(dict)) + 'h'
			lines = lines.replace(str(i), str(dict[i]))
	g.writelines(lines)
f.close()
g.close()
