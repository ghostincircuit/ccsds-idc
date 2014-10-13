#include "dwt.h"
#include <assert>
#include <stdlib.h>

static void dwt_all_rows(struct image *img, int rows, int cols, filter_t f)
{
        int i;
        for (i = 0; i < rows; i++) {
                (*f)(img->data + i*cols, cols)
        }
}

static void dwt_all_cols(struct image *img, int rows, int cols, filter_t f)
{
        int i;
        u32 *buf = malloc(rows*sizeof(img->cell_size));
        assert(buf);
        for (i = 0; i < cols; i++) {
                int j;
                for (j = 0; j < rows; j++)
                        buf[j] = img->data[i + j*cols];
                (*f)(buf, cols);
                for (j = 0; j < rows; j++)
                        img->data[i + j*cols] = buf[j];
        }
        free(buf);
}

void dwt2d(struct image *img, enum dwt_order order, int scale, filter_t f)
{
        assert(scale == 1 || scale == 2 || scale == 4 || scale == 8);
        assert(f);
        assert(img);
        assert(img->height&1 == 0);
        assert(img->width&1 == 0);
        int rows = img->height / scale;
        int cols = img->width / scale;
        
        if (order == ROW_FIRST) {
                dwt_all_rows(img, rows, cols, f);
                dwt_all_cols(img, rows, cols, f);
        } else {
                dwt_all_rows(img, rows, cols, f);
                dwt_all_cols(img, rows, cols, f);                
        }
}

void dwt_53i(u32 data[], int len)
{
}

void dwt_97i(u32 data[], int len)
{
}

void dwt_97f(float data[], int len)
{
}

void idwt_53i(u32 data[], int len);
void idwt_97i(u32 data[], int len);
void idwt_97f(float data[], int len);
