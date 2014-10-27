import sys
import struct
cnt = 0
f = open("56_40_16_2_l_num.raw", 'w')
for i in range(0, 40):
	for j in range(0, 56):
		acc = i * 100 + j
		sys.stdout.write("%04d "%(acc))
		f.write(struct.pack('H', acc))
	print("")

