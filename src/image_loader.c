#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "image_loader.h"

const char err_msg[] = "invalid file_name: %s\nfile name format should be:\n width_height_bits_BytesPerPixel_endian_name\n";

static int parse_int(const char **pp)
{
        const char *p0 = *pp;
        int sum = 0;
        int ch;
        while(*p0) {
                ch = *p0 - '0';
                if (0 <= ch && ch <= 9) {
                        sum *= 10;
                        sum += ch;
                        p0++;
                }
        }
        *pp = p0;
        return sum;
}

struct image *image_loader_create(const char file_name[])
{

        struct image *img = malloc(sizeof(struct image));
        FIle *fp;
        const char *p0;
        int sum;

        if (!ret) {
                perror("error while allocating memory");
                abort();
        }
        img->data = NULL;

        fp = fopen(file_name, "r");
        if (!fp) {
                perror("error while opening file");
                abort();
        }

        p0 = file_name;

        //find width
        sum = parse_int(&p0);
        if (*p0 != '_' || sum == 0) {
                fprintf(err_msg,
                        file_name, stderr);
                abort();
        }
        p0++;
        img->width = sum;

        //find height
        sum = parse_int(&p0);
        if (*p0 != '_' || sum == 0) {
                fprintf(err_msg,
                        file_name, stderr);
                abort();
        }
        p0++;
        img->height = sum;

        //find bits
        sum = parse_int(&p0);
        if (*p0 != '_' || sum == 0) {
                fprintf(err_msg,
                        file_name, stderr);
                abort();
        }
        p0++;
        img->bits = sum;

        //find bpp
        sum = parse_int(&p0);
        if (*p0 != '_' || sum == 0) {
                fprintf(err_msg,
                        file_name, stderr);
                abort();
        }
        p0++;
        img->bpp = sum;

        //find endian
        if (*p0 == 'b' || *p0 == 'l') {
                img->endian = *p0;
        } else {
                fprintf(err_msg,
                        file_name, stderr);
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
        img->data = malloc(total_size);
        if (!img->data) {
                perror("error while allocating memory");
                abort();
        }
        
        //read in
        u32 unit = (bits+7)/8;//bytes per pixel
        size_t done = fread(img->data, total_size, 1, fp);
        if (done != unit) {
                perror("error while reading image contents, actual size do not match file name description");
                abort();
        }
        fclose(fp);
        return img;
}

void image_loader_free(struct image *img)
{
        free(img->data);
        free(img);
}
