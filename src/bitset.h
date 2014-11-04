#pragma once

#include "utils.h"

#define BITSET_DEFAULT_CAP_BITS 64

struct bitset {
        u32 cap;
        u32 size;
        u32 *d;
};

struct bitset *bitset_new();
struct bitset *bitset_push_bit(struct bitset *bs, u32 bit);
struct bitset *bitset_push_bits(struct bitset *bs, u32 word[], u32 cnt);
struct bitset *bitset_con(struct bitset *l, struct bitset *r);
struct bitset *bitset_con_with_limit(struct bitset *l, struct bitset *r, u32 lim);
struct bitset *bitset_copy(struct bitset *src);
u8 *bitset_dump(u8 dst[], struct bitset *src, u32 n);
u32 bitset_size(struct bitset *bs);
void bitset_delete(struct bitset *bs);
u32 bitset_geti(struct bitset *bs, u32 idx);
void bitset_seti(struct bitset *bs, u32 idx, u32 bit);
void bitset_print(struct bitset *bs, u32 start, u32 len, u32 sep);
