#pragma once
#include "utils.h"
#include "image_loader.h"

#define BPE_BLOCK_N 8
#define BPE_BLOCK_SIZE (BPE_BLOCK_N * BPE_BLOCK_N)

struct bpe_block {
        u32 data[BPE_BLOCK_SIZE];
};

struct bpe_block *bpe_block_from_image(const struct image *img, u32 *ph, u32 *pw);
void bpe_block_print(struct bpe_block *b);
void bpe_block_init(struct bpe_block *b, struct bpe_block *p);
void bpe_block_init_n(struct bpe_block *b, struct bpe_block *p, u32 n);
