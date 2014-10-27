#pragma once
#include "utils.h"
#include "bpe_block.h"

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

void bpe_segment(const struct bpe_parameters *para,
                        struct bitset *dst,
                        const struct bpe_block *src,
                        u23 size);

void bpe_decode_segment(const struct bpe_parameters *para,
                        struct bpe_block *dst,
                        const struct bitset *src,
                        u32 start,
                        u32 *end,
                        u23 *size);
                

