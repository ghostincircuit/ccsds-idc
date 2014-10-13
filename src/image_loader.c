#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "image_loader.h"

const char err_msg[] = "invalid file_name: %s\nfile name format should be:\n width_height_bits_BytesPerPixel_endian_name\n";

static int parse_int(const char **pp)
{
        assert(p);
        const char *p0 = *pp;
        int sum = 0;
        int ch;
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
                                fread(p+j, 1, 1, fp);
                                buffer[i] = pixel;
                        }
                }
        } else {
                const int offset = img->bpp-1;
                for (i = 0; i < img->height * img->width; i++) {
                        u32 j;
                        u8 *p = (u8 *)&pixel;
                        pixel = 0;
                        for (j = 0; j < img->bpp; j++) {
                                fread(p+offset-j, 1, 1, fp);
                                buffer[i] = pixel;
                        }
                }                
        }
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
        return nimg;
}

void image_loader_free(struct image *img)
{
        free(img->data);
        free(img);
}

#define _UNIT_TEST_

#ifdef _UNIT_TEST_

int main()
{
        struct image *img = image_loader_create("../res/8_4_16_2_l_samp.raw");
        image_loader_free(img);
        return 0;
}

#endif//_UNIT_TEST_
