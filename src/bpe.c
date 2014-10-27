#include "bpe.h"
#include "image_loader.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static void bpe_copy_block_u32(u32 *dst, u32 di, u32 dj, u32 dw,
                               const u32 *src, u32 si, u32 sj, u32 sw,
                               u32 rows, u32 cols)
{
        int i,j;
        u32 *d;
        const u32 *s;
        dst = dst + di*dw + dj;
        src = src + si*sw + sj;
        for (i = 0; i < rows; i++) {
                d = dst;
                s = src;
                for (j = 0; j < cols; j++) {
                        *d = *s;
                        d++;
                        s++;
                }
                dst = dst + dw;
                src = src + sw;
        }
}

static void bpe_image_get_single_block(const struct image *img, struct bpe_block *bp,
                                       u32 row, u32 col)
{
        u32 ow = img->width;
        u32 oh = img->height;
        u32 iw = img->width / 8;
        u32 ih = img->height / 8;
        u32 coln = col + iw;//new col
        u32 rown = row + ih;//new row

        u32 x[3] = {coln, col, coln};
        u32 y[3] = {row, rown, rown};
        u32 xx[3] = {1, 0, 1};
        u32 yy[3] = {0, 1, 1};

        int n, i, j, ii, jj;

        u32 *bpdata = (u32 *)bp->data;
        const u32 *imgdata = (u32 *)img->data;
        bpdata[0] = image_getij_u32(img, row, col);//dc
        for (n = 0; n < 3; n++) {
                //pi
                j = x[n];
                i = y[n];
                ii = yy[n];
                jj = xx[n];
                bpdata[ii*BPE_BLOCK_N+jj] = image_getij_u32(img, i, j);
                //Ci
                bpe_copy_block_u32(bpdata, 2*ii, 2*jj, BPE_BLOCK_N,
                                   imgdata, 2*i, 2*j, ow,
                                   2, 2);
                //Hi
                bpe_copy_block_u32(bpdata, 4*ii, 4*jj, BPE_BLOCK_N,
                                   imgdata, 4*i, 4*j, ow,
                                   4, 4);
        }
}

struct bpe_block *bpe_blocks_from_image(const struct image *img, u32 *ph, u32 *pw)
{
        assert(img);
        assert((img->width & 7) == 0);
        assert((img->height & 7) == 0);
        u32 ow = img->width;
        u32 oh = img->height;
        u32 iw = img->width / 8;
        u32 ih = img->height / 8;
        struct bpe_block *ret = malloc(sizeof(struct bpe_block) * iw*ih);
        int i, j;
        int cnt = 0;
        for (i = 0; i < ih; i++) {
                for (j = 0; j < iw; j++) {
                        bpe_image_get_single_block(img, ret+cnt, i, j);
                        cnt++;
                }
        }
        *ph = ih;
        *pw = iw;
        return ret;
}

void bpe_block_print(struct bpe_block *b)
{
        int i, j;
        for (i = 0; i < BPE_BLOCK_N; i++) {
                for (j = 0; j < BPE_BLOCK_N; j++) {
                        printf("%+05d ", b->data[i*BPE_BLOCK_N + j]);
                }
                printf("\n");
        }
}

//#define _TEST_BPE_GET_BLOCK_
#ifdef _TEST_BPE_GET_BLOCK_

int main()
{
        struct image *img = image_loader_create("../res/56_40_16_2_l_num.raw");
        u32 w, u32 h;
        struct bpe_block *ret = bpe_image_to_blocks(img, &h, &w);
        bpe_print_block(ret+34);
        return 0;
}
#endif//_TEST_BPE_GET_BLOCK_
