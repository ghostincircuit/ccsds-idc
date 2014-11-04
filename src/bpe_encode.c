#include "bpe_encode.h"
#include <stdlib.h>
#include <assert.h>

static void bpe_encode_segment_find_width(
        const struct bpe_parameters *para,
        const struct bpe_block *src,
        u32 nblocks,
        u32 *dcwidth,
        u32 *acwidth)
{
}

static void bpe_encode_segment_header(
        const struct bpe_parameters *para,
        u32 is_first,
        u32 is_last,
        u8 segcnt,
        u32 dcwidth,
        u32 maxacwidth,
        struct bitset *dst)
{
}

static u32 bpe_encode_segment_findk(
        const struct bpe_parameters *para,
        const u32 quant[],
        u32 nblocks,
        u32 N)
{
        return 0;
}

static void bpe_encode_segment_diff(
        const struct bpe_parameters *para,
        u32 quant[],
        u32 nblocks,
        u32 N,
        u32 is_dc,
        struct bitset *dst)
{
}

static void bpe_encode_segment_dc(
        const struct bpe_parameters *para,
        const struct bpe_block *src,
        u32 nblocks,
        u32 dcwidth,
        u32 maxacwidth,
        struct bitset *dst)
{
}

static void bpe_encode_segment_acwidth(
        const struct bpe_parameters *para,
        u32 acwidth[],
        u32 nblocks,
        u32 N,
        struct bitset *dst)
{
}

static void bpe_encode_segment_ac(
        const struct bpe_parameters *para,
        const struct bpe_block *src,
        const u32 acwidth[],
        u32 nblocks,
        struct bitset *dst)
{
}


void bpe_encode_segment(const struct bpe_parameters *para,
                        const struct bpe_block *src,
                        u32 nblocks,
                        u8 segcnt,
                        u32 is_first,
                        u32 is_last,
                        struct bitset *dst)
{
        u32 limit = para->SegmentByteLimit * 8;
        u32 dcwidth;
        u32 *acwidth = malloc(sizeof(u32) * nblocks);
        assert(acwidth);
        bpe_encode_segment_find_width(
                para,
                src,
                nblocks,
                &dcwidth,
                acwidth);

        int i;
        u32 maxacwidth = 0;
        for (i = 0; i < nblocks; i++)
                if (maxacwidth < acwidth[i])
                        maxacwidth = acwidth[i];

        struct bitset *seg = bitset_new();
        bpe_encode_segment_header(
                para,
                is_first,
                is_last,
                segcnt,
                dcwidth,
                maxacwidth,
                seg);

        struct bitset *dc = bitset_new();
        bpe_encode_segment_dc(
                para,
                src,
                nblocks,
                dcwidth,
                maxacwidth,
                dc);
        bitset_con_with_limit(seg, dc, limit);
        bitset_delete(dc);

        struct bitset *bacwidth = bitset_new();
        bpe_encode_segment_acwidth(
                para,
                acwidth,
                nblocks,
                maxacwidth,
                bacwidth);
        bitset_con_with_limit(seg, bacwidth, limit);
        bitset_delete(bacwidth);

        struct bitset *ac = bitset_new();
        bpe_encode_segment_ac(
                para,
                src,
                acwidth,
                nblocks,
                ac);
        bitset_con_with_limit(seg, ac, limit);
        bitset_delete(ac);

        free(acwidth);
        bitset_con(dst, seg);
        bitset_delete(seg);
}

void bpe_encode_blocks(const struct bpe_parameters *para,
                       const struct bpe_block *src,
                       u32 nblocks,
                       struct bitset *dst)
{
        u32 S = para->S;
        u32 i;
        u32 step;
        u32 first = 1;
        u32 last = 0;
        u8 segcnt = 0;//use 32 because set_field expect 32bits alignment
        for (i = 0; i < nblocks; i += step) {
                if (i + S <= nblocks)
                        step = S;
                else
                        step = nblocks-i;
                if (i + step == nblocks)
                        last = 1;
                bpe_encode_segment(para,
                                   src+i,
                                   step,
                                   segcnt,
                                   first,
                                   last,
                                   dst);
                segcnt++;
                first = 0;
        }
}

#ifdef _TEST_BPE_ENCODE_
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
#endif
