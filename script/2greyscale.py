import sys
import skimage
import skimage.io
import argparse
from struct import pack

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="generated file name would be width_height_bits_bytesPerPixel_endian_name")
    parser.add_argument('bits', help='number of bits per pixel')
    parser.add_argument('bpp', help='number of bytes per pixel', choices=['1', '2', '4'])
    parser.add_argument('inf', help='input file name')
    parser.add_argument('--outf', '-o', help='output file name')
    parser.add_argument('--endian', '-e', help='endian, l or b', default='l', choices=['l', 'b'])

    ns = parser.parse_args()
    bits = int(ns.bits)
    bpp = int(ns.bpp)
    inf = ns.inf
    endian = ns.endian
    fp = open(inf, 'rb')
    data = skimage.io.imread(fp, as_grey=True)
    fp.close()
    height = len(data)
    width = len(data[0])
    full = (1<<bits)-1

    i = len(inf)-1
    while i >= 0:
        if inf[i] == '/':
            break
        else:
            i = i-1

    j = i+1
    while j < len(inf):
        if inf[j] == '.':
            break
        else:
            j = j+1

    if (ns.outf == None):
        outf = "%d_%d_%d_%d_%s_"%(width,height,bits,bpp,endian) + inf[i+1:j] + '.raw'
    else:
        outf = ns.outf

    if endian == 'l':
        fmt = '<'
    else:
        fmt = '>'

    if bpp == 1:
        fmt = fmt + 'B'
    elif bpp == 2:
        fmt = fmt + 'H'
    else:
        fmt = fmt + 'I'


    ofp = open(outf, 'wb')
    for i in data:
        for j in i:
            packed = pack(fmt, int(j*full))
            ofp.write(packed)

    ofp.close()
    
