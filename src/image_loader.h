#pragma once
#include "utils.h"

struct image {
        u32 width;
        u32 length;
        u32 bits;
        u32 bpp;//bytes per pixel
        char endian;//'b' means big endian, 'l' means little endian
        char file_name[32];
        void *data;

};

//format of file name should be:width_height_bits_name
//for example:1024_768_8_denglx
struct image *image_loader_create(const char file[]);
void image_loader_free(struct image *img);
