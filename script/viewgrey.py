#!/usr/bin/env python
import sys
import skimage
import skimage.io
import argparse
import numpy as np
import struct
import matplotlib.pyplot as plt

def next_word(a, start):
    end = len(a)
    i = start
    #print("here: %s"%(a[start:]))
    while i != end and a[i] != '_':
        i = i+1
    if i == end:
        return False, 0
    return True, i

def file_name_error():
    sys.stderr.write("file name not correct\n")
    sys.exit(-1)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="show a greyscale image in raw(our) format")
    parser.add_argument('name', help='name of data file')
    ns = parser.parse_args()
    file_name = ns.name

    i = len(file_name)-1
    while i >= 0 and file_name[i] != '/':
        i -= 1
    i += 1
    file_name = file_name[i:]

    i = 0
    #print(file_name)
    flag, ni = next_word(file_name, i)
    if not flag:
        file_name_error()
    width = int(file_name[i:ni])
    i = ni+1

    flag, ni = next_word(file_name, i)
    if not flag:
        file_name_error()
    height = int(file_name[i:ni])
    i = ni+1

    flag, ni = next_word(file_name, i)
    if not flag:
        file_name_error()
    bits = int(file_name[i:ni])
    i = ni+1
    full = (1<<bits)-1

    flag, ni = next_word(file_name, i)
    bpp = int(file_name[i:ni])
    if not flag:
        file_name_error()
    i = ni+1
    
    flag, ni = next_word(file_name, i)
    endian = file_name[i:ni]
    if not flag:
        file_name_error()
    i = ni+1

    name = file_name[i:]

    if endian == 'b':
        fmt = '>'
    else:
        fmt = '<'
    if bpp == 1:
        fmt += 'B'
    elif bpp == 2:
        fmt += 'H'
    else:
        fmt += 'I'

    fp = open(file_name, 'rb')
    img = np.empty((height, width), np.float64)
    for i in range(0, height):
        for j in range(0, width):
            pixel = fp.read(bpp)
            pixel = struct.unpack(fmt, pixel)[0]
            img[i][j] = float(pixel)/full    

    a = plt.figure()
    a.suptitle(file_name)
    plt.imshow(img, cmap='gray', interpolation='nearest')
    a.show()
    sys.stderr.write("press any key to quit\n")
    raw_input()
