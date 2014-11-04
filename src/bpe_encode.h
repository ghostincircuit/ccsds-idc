#pragma once
#include "utils.h"
#include "bpe_block.h"
#include "bitset.h"

struct bpe_parameters {
        u32 SegmentByteLimit;
        u32 DCStop;
        u32 BitPlaneStop;
        u32 StageStop;
        u32 UseFill;
        /*
          segment size in blocks. i.e. blocks per segment,
          may be not times of 16 for last segment
        */
        u32 S;
        u32 OptDCSelect;
        u32 OptACSelect;
        u32 ImageWidth;
        u32 ImageHeight;

        u32 BitShiftLL3;

};

void bpe_encode_blocks(const struct bpe_parameters *para,
                       const struct bpe_block *src,
                       u32 nblocks,
                       struct bitset *dst);

void bpe_encode_segment(const struct bpe_parameters *para,
                        const struct bpe_block *src,
                        u32 nblocks,
                        u8 segcnt,
                        u32 is_first,
                        u32 is_last,
                        struct bitset *dst);
