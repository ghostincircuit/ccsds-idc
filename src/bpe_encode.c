#include "bpe_encode.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
static u32 bitwidth_2c(u32 x)
{
        u32 prev = !!(x & (1u<<31));
        u32 cur;
        int i;
        for (i = 30; i > 0; i--) {
                cur = !!((1<<i) & x);
                if (cur != prev)
                        break;
        }
        return i + 2;
}

static u32 bitwidth_2s(u32 x)
{
        s32 xx = *(s32 *)&x;
        if (xx < 0)
                xx = -x;
        return bitwidth_2c(xx);
}

static void bpe_encode_segment_find_width(
        const struct bpe_parameters *para,
        const struct bpe_block *src,
        u32 nblocks,
        u32 *dcwidth,
        u32 *acwidth)
{
        //find dcwidth
        int i, j;
        u32 width = 0;
        for (i = 0; i < nblocks; i++) {
                u32 t = bitwidth_2c(src[i].data[0]);
                if (width < t)
                        width = t;
        }
        *dcwidth = width;

        //find acwidth[i]
        for (i = 0; i < nblocks; i++) {
                acwidth[i] = 0;
                for (j = 1; j < 64; j++) {
                        u32 t = bitwidth_2s(src[i].data[j]);//sign already counted
                        if (t > acwidth[i])
                                acwidth[i] = t;
                }
        }
}
static void fill_field(u32 *word,
                       u32 start,
                       u32 len,
                       u32 data)
{
        u32 mask = (1u<<len)-1;
        if (len == 32)
                mask = 0xffffffff;
        mask = mask << start;
        data = data << start;
        *word &= ~mask;
        *word |= data;
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
        //note that the length vary according to is_last
        u32 header = 0;
        u32 as = 24;//actual size

        if (is_first)
                header |= 1;

        if (is_last) {
                header |= 2;
                as = 32;
        }

        fill_field(&header, 2, 8, segcnt);

        fill_field(&header, 10, 5, dcwidth);

        fill_field(&header, 15, 5, maxacwidth);

        bitset_push_bits(dst, &header, as);
}

static void bpe_encode_segment_diff_gaggle(
        const u32 quant[],
        u32 nblocks,
        s32 k,
        u32 N,
        struct bitset *dst)
{
        int i, j;
        u32 d,q;
        if (k < 0) {//uncoded
                for (i = 0; i < nblocks; i++) {
                        bitset_push_bits(dst, quant+i, N);
                }
                return;
        }
        //else
        for (i = 0; i < nblocks; i++) {
                d = quant[i] >> k;
                for (j = 0; j < d; j++)
                        bitset_push_bit(dst, 0);
                bitset_push_bit(dst,1);
        }

        for (i = 0; i < nblocks; i++) {
                q = quant[i];
                bitset_push_bits(dst, &q, k);
        }
}

static u32 bpe_encode_segment_diff_gaggle_try(
        const u32 quant[],
        u32 nblocks,
        s32 k,
        u32 N)
{
        int i;
        u32 d, data;
        if (k < 0) {//uncoded
                return nblocks * N;
        }
        //else
        u32 sum = 0;
        for (i = 0; i < nblocks; i++) {
                data = ((1<<N)-1) & quant[i];
                d = data >> k;
                sum += (d + 1 + k);
        }
        return sum;
}

static s32 bpe_encode_segment_findk_opt(
        const struct bpe_parameters *para,
        const u32 quant[],
        u32 nblocks,
        u32 N)
{
        int bestk;
        int lk;
        u32 bestcost = 0xffffffff;
        if (N == 2)
                lk = 0;
        else if (2 < N && N <= 4)
                lk = 2;
        else if (4 < N && N <= 8)
                lk = 6;
        else if (8 < N && N <= 10)
                lk = 8;
        else
                assert(0);//uncaught N

        int i;
        u32 sz;
//#define WATCH_K
#ifdef WATCH_K
        printf("===========================\n");
#endif
        for (i = -1; i <= lk; i++) {
                sz = bpe_encode_segment_diff_gaggle_try(
                        quant,
                        nblocks,
                        i,
                        N);
                if (sz < bestcost) {
                        bestcost = sz;
                        bestk = i;
                }
#ifdef WATCH_K
                printf("k=%d: %d\n", i, sz);
#endif
        }
#ifdef WATCH_K
        printf("===========================\n");
#endif
        return bestk;
}

static s32 bpe_encode_segment_findk_heu(
        const struct bpe_parameters *para,
        const u32 quant[],
        u32 nblocks,
        u32 N)
{
        u32 delta = 0;
        u32 i;
        for (i = 0; i < nblocks; i++)
                delta += quant[i];
        u32 J = nblocks;

        if (64 * delta >= 23 * J * (1<<N))
                return -1;
        if (207 * J > 128 * delta)
                return 0;
        if (J * (1<<(N-5)) <= (128 * delta + 49 * J))
                return N-2;
        for (i = N-2; i > 0; i--)
                if (J * (1<<(i+7)) <= (128 * delta + 49 * J))
                        break;
        return i;
}

static s32 bpe_encode_segment_findk(
        const struct bpe_parameters *para,
        const u32 quant[],
        u32 nblocks,
        u32 N,
        u32 is_opt)
{
        s32 opt = bpe_encode_segment_findk_opt(
                para,
                quant,
                nblocks,
                N);

        s32 heu = bpe_encode_segment_findk_heu(
                para,
                quant,
                nblocks,
                N);
        if (para->Other)
                return -1;
        if (is_opt)
                return opt;
        else
                return heu;
}

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define ABS(x) (((x) < 0) ? -(x) : (x))
static void bpe_encode_segment_diff_diff(
        const struct bpe_parameters *para,
        u32 data[],
        u32 nblocks,
        u32 N,
        u32 is_dc,
        u32 buff[])
{
        s32 thetam;
        s32 xmin = is_dc ? -(1<<(N-1)) : 0;
        s32 xmain = is_dc ? (1<<(N-1)) - 1 : (1<<N)-1;
        int i;
        for (i = 0; i < nblocks; i++) {
                s32 det = data[i] - data[i-1];
                thetam = MIN(data[i-1] - xmin, xmain - data[i-1]);
                if (0 <= det && det <= thetam)
                        buff[i] = 2*det;
                else if (-thetam <= det && det < 0)
                        buff[i] = -2*det-1;
                else
                        buff[i] = thetam + ABS(det);
        }
}

static void bpe_encode_segment_diff(
        const struct bpe_parameters *para,
        u32 quant[],
        u32 nblocks,
        u32 N,
        u32 is_dc,
        struct bitset *dst)
{
        int i;
        const u32 Ntobits[] = {1,1,1,2,2,3,3,3,3,4,4};
        assert(2<= N && N <= 10);//N range
        if (N == 0)
                return;
        if (N == 1) {
                for (i = 0; i < nblocks; i++)
                        bitset_push_bit(dst, quant[i]);
        }
        //first gaggle is special
        s32 k;
        u32 codelen;
        u32 diffs[16];
        u32 is_opt = is_dc ? para->OptDCSelect : para->OptACSelect;
        assert(nblocks >= 16);
        bpe_encode_segment_diff_diff(para, quant+1, 15, N, is_dc, diffs);
        k = bpe_encode_segment_findk(para, diffs, 15, N, is_opt);
        codelen = Ntobits[N];
        bitset_push_bits(dst, (u32 *)&k, codelen);//-1=0xffffff, so it is ok
        bitset_push_bits(dst, quant, N);//first N refernce
        bpe_encode_segment_diff_gaggle(
                diffs,
                15,
                k,
                N,
                dst);
        u32 step;
        for (i = 16; i != nblocks; i += step) {
                step = nblocks - i;
                if (step > 16)
                        step = 16;
                bpe_encode_segment_diff_diff(para, quant+i, step, N, is_dc, diffs);
                k = bpe_encode_segment_findk(para, diffs, step, N, is_opt);
                bitset_push_bits(dst, (u32 *)&k, codelen);
                bpe_encode_segment_diff_gaggle(
                        diffs,
                        step,
                        k,
                        N,
                        dst);
        }
}

#define MAX(x, y) ((x) > (y) ? (x) : (y))
static void bpe_encode_segment_dc(
        const struct bpe_parameters *para,
        const struct bpe_block *src,
        u32 nblocks,
        u32 dcwidth,
        u32 maxacwidth,
        struct bitset *dst)
{
        //calc q
        u32 q_, q, i;
        s32 stat = dcwidth - (1 + maxacwidth/2);
        if (dcwidth <= 3)
                q_ = 0;
        else if (stat <= 1 && dcwidth > 3)
                q_ = dcwidth - 3;
        else if (stat > 10 && dcwidth > 3)
                q_ = dcwidth - 10;
        else
                q_ = 1 + maxacwidth/2;
        q = MAX(q_, para->BitShiftLL3);
        u32 *quant = malloc(sizeof(u32)*nblocks);
        assert(quant);
        for (i = 0; i < nblocks; i++)
                quant[i] = src[i].data[0] >> q;
        u32 N = MAX(dcwidth-q, 1);
        bpe_encode_segment_diff(para, quant, nblocks, N, 1, dst);
        for (i = 0; i < nblocks; i++)
                quant[i] = src[i].data[0];
        for (i = 0; i< nblocks; i++)
                bitset_push_bits(dst, quant+i, q);
}

static void bpe_encode_segment_acwidth(
        const struct bpe_parameters *para,
        u32 acwidth[],
        u32 nblocks,
        u32 N,
        struct bitset *dst)
{
        bpe_encode_segment_diff(para, acwidth, nblocks, N, 0, dst);
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
#include <stdio.h>

void test_bitwidth()
{
        int ii;
        printf("=========== complement ==========================\n");
        for (ii = -4; ii <= 4; ii++)
                printf("%d:%d\n", ii, bitwidth_2c(ii));
        printf("=========== sign ==========================\n");
        for (ii = -4; ii <= 4; ii++)
                printf("%d:%d\n", ii, bitwidth_2s(ii));
}

int main()
{
        struct bpe_parameters para = {
                .SegmentByteLimit = 1024,
                .DCStop = 1,
                .BitPlaneStop = 0,
                .StageStop = 0,
                .UseFill = 1,
                .S = 32,
                .OptDCSelect = 1,
                .OptACSelect = 1,
                .ImageWidth = 1024,
                .ImageHeight = 1024,
                .BitShiftLL3 = 3,
                .Other = 0,
        };
        const int NBLOCKS = 36;
        struct bpe_block src[NBLOCKS];
        struct bitset *dst = bitset_new();
        int i;
        for (i = 0; i < 64; i++) {
                src[0].data[i] = i;
        }
        bpe_block_init_n(src+1, src, NBLOCKS-1);
        for (i = 0; i < NBLOCKS; i++) {
                src[i].data[0] = (i*i*i+7)%256;
        }
        //bpe_block_print(src+NBLOCKS-1);

        bpe_encode_segment(
                &para,
                src,
                NBLOCKS,
                0,
                1,
                1,
                dst);

        printf("compressed size: %d\n", bitset_size(dst));
        bitset_delete(dst);
        return 0;
}
#endif
