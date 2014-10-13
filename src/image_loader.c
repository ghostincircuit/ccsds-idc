#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <error.h>

#include "image_loader.h"

const char err_msg[] = "invalid file_name: %s\nfile name format should be:\n width_height_bits_BytesPerPixel_endian_name\n";

static int parse_int(const char **pp)
{

        const char *p0 = *pp;
        int sum = 0;
        int ch;
        assert(pp);
        assert(p0);
        while(*p0) {
                ch = *p0 - '0';
                if (0 <= ch && ch <= 9) {
                        sum *= 10;
                        sum += ch;
                        p0++;
                } else
                        break;
        }
        *pp = p0;
        return sum;
}

struct image *image_loader_create(const char file_name[])
{
        assert(file_name);
        struct image *img = malloc(sizeof(struct image));
        FILE *fp;
        const char *p0;
        int sum;

        if (!img) {
                perror("error while allocating memory");
                abort();
        }
        img->data = NULL;
        img->cell_size = 4;

        fp = fopen(file_name, "r");
        if (!fp) {
                perror("error while opening file");
                abort();
        }

        p0 = file_name + strlen(file_name) - 1;
        while (*p0 != '/')
                p0--;
        p0++;

        //find width
        sum = parse_int(&p0);
        if (*p0 != '_' || sum == 0) {
                fprintf(stderr, err_msg, file_name);
                abort();
        }
        p0++;
        img->width = sum;

        //find height
        sum = parse_int(&p0);
        if (*p0 != '_' || sum == 0) {
                fprintf(stderr, err_msg, file_name);
                abort();
        }
        p0++;
        img->height = sum;

        //find bits
        sum = parse_int(&p0);
        if (*p0 != '_' || sum == 0) {
                fprintf(stderr, err_msg, file_name);
                abort();
        }
        p0++;
        img->bits = sum;

        //find bpp
        sum = parse_int(&p0);
        if (*p0 != '_' || sum == 0) {
                fprintf(stderr, err_msg, file_name);
                abort();
        }
        p0++;
        img->bpp = sum;

        //find endian
        if (*p0 == 'b' || *p0 == 'l') {
                img->endian = *p0;
        } else {
                fprintf(stderr, err_msg, file_name);
                abort();
        }
        //file_name
        int copied = 0;
        while(*p0 && copied != 31) {
                img->file_name[copied] = *p0;
                p0++;
                copied++;
        }
        img->file_name[copied] = 0;

        //allocate memoroy
        u32 total_size = img->bpp * img->height * img->width;
        u32 buffer_size = sizeof(img->cell_size) * img->height * img->width;
        img->data = malloc(buffer_size);
        if (!img->data) {
                perror("error while allocating memory");
                abort();
        }

        //determine size
        fseek(fp, 0, SEEK_END);
        u32 file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (file_size < total_size) {
                fprintf(stderr, "file size smaller than described by file name\n");
                abort();
        }

        //read in
        u32 i;
        u32 pixel;

        u32 *buffer = img->data;
        if (img->endian == 'l') {
                for (i = 0; i < img->height * img->width; i++) {
                        u32 j;
                        u8 *p = (u8 *)&pixel;
                        pixel = 0;
                        for (j = 0; j < img->bpp; j++) {
                                assert(fread(p+j, 1, 1, fp));
                        }
                        if (p[j] &0x80) {
                                for (j = img->bpp; j != 4; j++) {
                                        p[j] = 0xff;
                                }
                        }
                        buffer[i] = pixel;
                }
        } else {
                const int offset = img->bpp-1;
                for (i = 0; i < img->height * img->width; i++) {
                        u32 j;
                        u8 *p = (u8 *)&pixel;
                        pixel = 0;
                        for (j = 0; j < img->bpp; j++) {
                                assert(fread(p+offset-j, 1, 1, fp));
                        }
                        if (p[offset] & 0x80) {
                                for (j = img->bpp; j != 4; j++) {
                                        p[j] = 3;
                                }
                        }
                        buffer[i] = pixel;
                }
        }
        //after that, change the endian. We always use little endian inside
        img->endian = 'l';
        fclose(fp);
        return img;
}

struct image *image_loader_copy(struct image *img)
{
        struct image *nimg = malloc(sizeof(struct image));
        memcpy(nimg, img, sizeof(struct image));
        nimg->data = malloc(nimg->width * nimg->height * sizeof(nimg->cell_size));
        if (!nimg->data) {
                perror("error while copying image");
                abort();
        }
        int bytes = nimg->width * nimg->height * nimg->cell_size;
        memcpy(nimg->data, img->data, bytes);
        return nimg;
}

void image_loader_free(struct image *img)
{
        free(img->data);
        free(img);
}

//#define _UNIT_TEST_IMAGE_LOADER_

void image_loader_log(struct image *img)
{
        int i;
        int cols = img->width;
        int rows = img->height;

        int ppr = 8;//pixel per row
        u32 *dat = img->data;

        int cnt = ppr;
        for (i = 0; i < cols*rows; i++) {
                if (cnt == ppr) {
                        printf("%04d-%04d: ", i, i+ppr-1);
                        cnt = 0;
                }
                printf("%08x ", dat[i]);
                cnt++;
                if (cnt == ppr)
                        printf("\n");

        }
}

void image_loader_save(struct image *img, const char name[], const char path[])
{
        const int N = 256;
        char file_name[N];
        int i;
        int cols = img->width;
        int rows = img->height;
        int bits = img->bits;
        int bpp = img->bpp;
        char endian = 'l';
        FILE *fp;
        assert(name);
        assert(path);
        assert(img);
        snprintf(file_name, N, "%d_%d_%d_%d_%c_%s",
                 cols, rows, bits, bpp, endian, name);

        fp = fopen(file_name, "wb");
        if (fp == NULL) {
                perror("error while opening file \n");
                abort();
        }
        u32 *dat = img->data;
        size_t ret;
        for (i = 0; i < cols*rows; i++) {
                if (bpp == 1) {
                        u8 b = (u8)dat[i];
                        u32 prefix = dat[i] & 0xffffff80;
                        //if this assertion does not stand true, there's overflow
                        assert(prefix == 0xffffff80 || prefix == 0);
                        ret = fwrite(&b, 1, 1, fp);
                } else if (bpp == 2) {
                        u16 h = (u16)dat[i];
                        u32 prefix = dat[i] & 0xffff8000;
                        //if this assertion does not stand true, there's overflow
                        assert(prefix == 0xffff8000 || prefix == 0);
                        ret = fwrite(&h, 1, 2, fp);
                } else {
                        u32 x = (u32)dat[i];
                        ret = fwrite(&x, 1, 4, fp);
                }
                assert(ret);
        }
        fclose(fp);
}

void image_loader_assert_equal(struct image *img1, struct image *img2)
{
        assert(img1->width == img2->width);
        assert(img1->height == img2->height);
        //assert(img1->bpp == img2->bpp);
        //assert(img1->bits == img2->bits);
        assert(img1->cell_size == img2->cell_size);
        int cols = img1->width;
        int rows = img1->height;

        u32 *d1 = img1->data;
        u32 *d2 = img2->data;

        int i,j;
        for (i = 0; i < rows; i++) {
                for (j = 0; j < cols; j++) {
                        int idx = j + i*cols;
                        assert(d1[idx] == d2[idx]);
                }
        }
}

//#define _UNIT_TEST_IMAGE_LOADER_
#ifdef _UNIT_TEST_IMAGE_LOADER_

int main()
{
        struct image *img = image_loader_create("../res/8_4_16_2_l_samp.raw");
        image_loader_save(img, "sssamp.raw", "./");
        image_loader_free(img);
        return 0;
}

#endif//_UNIT_TEST_IMAGE_LOADER_
