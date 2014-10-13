#pragma once
#include "utils.h"

struct image {
        u32 width;
        u32 height;
        u32 bits;
        u32 bpp;//bytes per pixel
        char endian;//'b' means big endian, 'l' means little endian
        char file_name[32];
        void *data;
        u32 cell_size;//bufferpixel size in bytes
};

//format of file name should be:width_height_bits_name
//for example:1024_768_8_denglx
struct image *image_loader_create(const char file[]);
struct image *image_loader_copy(struct image *img);
void image_loader_save(struct image *img, const char name[], const char path[]);
void image_loader_free(struct image *img);
void image_loader_log(struct image*img);
void image_loader_assert_equal(struct image *img1, struct image *img2);
