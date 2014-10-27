#pragma once
#include "utils.h"
#include "image_loader.h"

#define BPE_BLOCK_N 8
#define BPE_BLOCK_SIZE (BPE_BLOCK_N * BPE_BLOCK_N)

struct bpe_block {
        u32 data[BPE_BLOCK_SIZE];
};

struct bpe_block *bpe_image_to_blocks(const struct image *img, u32 *ph, u32 *pw);

struct bpe_segment {
        struct bitset *dc;
        struct bitset *ac_width;
        struct bitset *stage[4];
};


struct bpe_parameters {
        u32 SegmentByteLimit;
        u32 DCStop;
        u32 BitPlaneStop;
        u32 StageStop;
        u32 UseFill;
        u32 S;//segment size in blocks. i.e. blocks per segment, may be not times of 16 for last segment
        u32 OptDCSelect;
        u32 OptACSelect;
};

void bpe_encode_segment(const struct bpe_parameters *para,
                        struct bitset *dst,
                        const struct bpe_block *src,
                        u23 size);

void bpe_decode_segment(const struct bpe_parameters *para,
                        struct bpe_block *dst,
                        const struct bitset *src,
                        u32 start,
                        u32 *end,
                        u23 *size);
                
