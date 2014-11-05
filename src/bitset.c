#include "bitset.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static const u32 cell_width_bits = sizeof(u32) * 8;

struct bitset *bitset_new()
{
        struct bitset *bs = malloc(sizeof(struct bitset));
        assert(bs);
        bs->d = malloc(BITSET_DEFAULT_CAP_BITS / 8);
        bs->cap = BITSET_DEFAULT_CAP_BITS;
        bs->size = 0;
        return bs;
}

struct bitset *bitset_push_bit(struct bitset *bs, u32 bit)
{
        u32 *n;
        assert(bs);
        if (bs->cap == bs->size) {
                n = realloc(bs->d, bs->size / 8 * 2);
                assert(n);
                bs->d = n;
                bs->cap = bs->cap*2;
        }
        bit = bit ? 1 : 0;
        u32 byte = bs->size / cell_width_bits;
        u32 offset = bs->size % cell_width_bits;
        u32 mask = bit<<offset;
        bs->d[byte] |= mask;
        bs->size++;
        return bs;
}

struct bitset *bitset_push_bits(struct bitset *bs, const u32 word[], u32 cnt)
{
        int i;
        u32 byte, offset;
        u32 bit;
        assert(bs);
        assert(word);
        for (i = 0; i < cnt; i++) {
                byte = i / cell_width_bits;
                offset = i % cell_width_bits;
                bit = word[byte] & (1<<offset);
                bitset_push_bit(bs, bit);
        }
        return bs;
}

struct bitset *bitset_con(struct bitset *l, const struct bitset *r)
{
        assert(l);
        assert(r);
        bitset_push_bits(l, r->d, r->size);
        return l;
}

struct bitset *bitset_con_with_limit(struct bitset *l, const struct bitset *r, u32 lim)
{
        assert(l);
        assert(r);
        assert(l->size <= lim);
        u32 avail = lim - l->size;
        u32 rsz = r->size;
        u32 todo = rsz < avail ? rsz : avail;
        bitset_push_bits(l, r->d, todo);
        return l;
}

struct bitset *bitset_copy(const struct bitset *src)
{
        u32 n = src->size;
        u32 copy_size = (n + cell_width_bits-1)/cell_width_bits*(cell_width_bits/8);
        struct bitset *dst = malloc(sizeof(struct bitset));
        dst->size = n;
        dst->d = malloc(copy_size);
        memcpy(dst->d, src->d, copy_size);
        return dst;
}

u8 *bitset_dump(u8 dst[], const struct bitset *src, u32 n)
{
        n = (n + 7) / 8;
        n = src->size > n ? n : src->size;
        memcpy(dst, src->d, n);
        return dst;
}

u32 bitset_size(struct bitset *bs)
{
        return bs->size;
}

void bitset_delete(struct bitset *bs)
{
        free(bs->d);
        free(bs);
}

u32 bitset_geti(struct bitset *bs, u32 idx)
{
        assert(idx < bs->size);
        u32 byte = idx / cell_width_bits;
        u32 offset = idx % cell_width_bits;
        return (bs->d[byte] & (1<<offset)) ? 1 : 0;
}

void bitset_seti(struct bitset *bs, u32 idx, u32 bit)
{
        assert(idx < bs->size);
        bit = bit ? 1 : 0;
        u32 byte = idx / cell_width_bits;
        u32 offset = idx % cell_width_bits;
        bs->d[byte] = bs->d[byte] | (bit<<offset);
}

void bitset_print(struct bitset *bs, u32 start, u32 len, u32 sep)
{
        int i;
        int cnt = 0;
        for (i = 0; i < len; i++) {
                if (start + i >= bs->size)
                        break;
                u32 bit = bitset_geti(bs, start + i);
                printf("%d", bit);
                cnt++;
                if (cnt == sep) {
                        cnt = 0;
                        printf(" ");
                }
        }
        printf("\n");
}

//#define _TEST_BITSET_
#ifdef _TEST_BITSET_

#include <stdio.h>

int main()
{
        u32 w = 0x00ff55aa;
        int i;
        struct bitset *bs = bitset_new();
        bitset_push_bits(bs, &w, 32);
        for (i = 0; i < bs->size; i++) {
                printf("%d", bitset_geti(bs, i));
        }
        printf("\n");

        u32 ww[] = {0x11111111, 0x12345678, 0x22222222, 0x12345678};
        u32 www[4];
        struct bitset *bbss = bitset_new();
        bitset_push_bits(bbss, ww, sizeof(ww)*8);
        bitset_dump((u8 *)www, bbss, sizeof(ww)*8);
        for (i = 0; i < 4; i++)
                printf("%08x ", www[i]);
        printf("\n");
        return 0;
}
#endif//_TEST_BITSET_
