#         1         2         3         4         5         6         7         8         9         0         1         2         3
#1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
codes='''
710000000100011000111110000000000000000000000110000000000100110006
710000000100011000111110000000000000000000000110000000000101001106
'''

codes = codes.replace(' ', '')
codes = [int(code[1:-1].strip().replace(' ', ''), 2) for code in codes.split()]

result = 0

for code1 in codes:
	for code2 in codes:
		result |= code1 ^ code2

result = str(bin(result))[2:].rjust(65, '0') + '0'

print result

pos = result.find('1')
first_pos = pos
prev_pos = pos
while pos > -1:
	pos = result.find('1', pos+1)

	if pos - prev_pos > 1:
		print first_pos, '-', prev_pos
		first_pos = pos

	if pos > -1:
		prev_pos = pos

print first_pos, '-', prev_pos
