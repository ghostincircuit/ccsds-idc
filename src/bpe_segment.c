#include "bpe_segment.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void bpe_encode_segment_find_width(
        u32 *dc_width,
        u32 *ac_width,
        const struct bpe_block *src,
        u32 nblocks)
{
}

static void bpe_encode_segment_encode_header(
        u32 dc_width,
        u32 *ac_width,
        const struct bpe_parameters *para,
        struct bitset *dst,
        u32 first)
{
}

static void bpe_encode_segment_encode_dc(
        u32 dc_width,
        u32 *ac_width,
        const struct bpe_parameters *para,
        struct bitset *dst,
        const struct bpe_block *src,
        u32 nblocks)
{
}

static void bpe_encode_segment_encode_acwidth(
        u32 *ac_width,
        const struct bpe_parameters *para,
        struct bitset *dst,
        u32 nblocks)
{
}

static void bpe_encode_segment_encode_ac(
        u32 *ac_width,
        const struct bpe_parameters *para,
        struct bitset *dst,
        const struct bpe_block *src,
        u32 nblocks)
{
}


void bpe_encode_segment(const struct bpe_parameters *para,
                        struct bitset *dst,
                        const struct bpe_block *src,
                        u32 nblocks,
                        u32 first)
{
        u32 dc_width;
        u32 *ac_width = malloc(sizeof(u32) * nblocks);
        assert(ac_width);
        bpe_encode_segment_find_width(
                &dc_width,
                ac_width,
                src,
                nblocks);

        bpe_encode_segment_encode_header(
                dc_width,
                ac_width,
                para,
                dst,
                first);

        bpe_encode_segment_encode_dc(
                dc_width,
                ac_width,
                para,
                dst,
                src,
                nblocks);

        bpe_encode_segment_encode_acwidth(
                ac_width,
                para,
                dst,
                nblocks);

        bpe_encode_segment_encode_ac(
                ac_width,
                para,
                dst,
                src,
                nblocks);
        free(ac_width);
}

void bpe_encode_blocks(const struct bpe_parameters *para,
                       struct bitset *dst,
                       const struct bpe_block *src,
                       u32 nblocks)
{
        u32 gsz = para->S;
        u32 i;
        u32 step;
        u32 first = 1;
        for (i = 0; i < nblocks; i += step) {
                if (i + gsz <= nblocks)
                        step = gsz;
                else
                        step = nblocks-i;
                bpe_encode_segment(para, dst, src+i, gsz, first);
                first = 0;
        }
}

void bpe_decode_segment(const struct bpe_parameters *para,
                        struct bpe_block *dst,
                        const struct bitset *src,
                        u32 start,
                        u32 *end,
                        u32 *nblocks,
                        u32 *is_first,
                        u32 *is_last)
{
}

struct bpe_block *bpe_decode_blocks(const struct bpe_parameters *para,
                                    const struct bitset *src,
                                    u32 start,
                                    u32 *end,
                                    u32 *nblocks)
{
        u32 is_last, is_first;
        u32 cend;
        u32 csize;
        u32 totalblocks = para->ImageWidth * para->ImageHeight;
        struct bpe_block *ret = malloc(sizeof(struct bpe_block) * totalblocks);
        struct bpe_block *dst = ret;
        u32 first = 1;
        do {
                bpe_decode_segment(
                        para,
                        dst,
                        src,
                        start,
                        &cend,
                        &csize,
                        &is_first,
                        &is_last);
                start = cend;
                dst += csize;
                if (first) {
                        //check last followed by first
                        assert(is_first);
                        first = 0;
                }

        } while (!is_last);
        *nblocks = totalblocks;
        assert(totalblocks == (dst - ret));
        return ret;
}

#ifdef _TEST_BPE_SEGMENT_

int main()
{
        struct bpe_parameters para = {
                .SegmentByteLimit = 1024,
                .DCStop = 1,
                .BitPlaneStop = 0,
                .StageStop = 0,
                .UseFill = 1,
                .S = 32,
                .OptDCSelect = 0,
                .OptACSelect = 0,
                .ImageWidth = 1024,
                .ImageHeight = 1024
        };
        struct bpe_block src[32];
        struct bitset *dst = bitset_new();
        int i;
        for (i = 0; i < 64; i++) {
                src[0].data[i] = i;
        }
        bpe_block_init_n(src+1, src, 31);
        for (i = 0; i < 64; i++) {
                src[i].data[0] = 2 * i;
        }
        bpe_block_print(src+2);
///*
        bpe_encode_segment(
                &para,
                dst,
                src,
                16,
                1);
//*/
        return 0;
}
#endif//_TEST_BPE_SEGMENT_
