#pragma once
#include "utils.h"

void bpe_encode_diff(u32 N, struct bitset *dst, const u32 dcs[],
                     u32 nblocks, const struct bpe_parameters *para, u32 is_dc);
