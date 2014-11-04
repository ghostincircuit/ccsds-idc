#include "bpe_encode_diff.h"
#include <stdlib.h>
#include <assert.h>

static u32 bpe_encode_selectk_opt(const u32 diffs[], u32 n, u32 is_dc)
{
        return 0;
}

static u32 bpe_encode_selectk_heu(const u32 diffs[], u32 n, u32 J, u32 N)
{
}

void bpe_encode_diff(
        u32 N, struct bitset *dst,
        const u32 dcs[],
        u32 nblocks,
        const struct bpe_parameters *para,
        u32 is_dc)
{
        int i;
        int flag;
        if (N == 1) {
                for (i = 0; i < nblocks; i++)
                        bitset_push_bit(dst, dcs[i]&1);
                return;
        }

        u32 *diffs = malloc(sizeof(u32)*nblocks);
        for (i = N-1; i > 0; i--)
                diffs[i]= dcs[i] - dcs[i-1];
        diffs[0] = dcs[0];
        if (is_dc)
                flag = para->OptDCSelect;
        else
                flag = para->OptACSelect;
        u32 step;
        for (i = 0; i < nblocks; i+=step) {
                step = nblocks - i;
                if (step 
        }
        assert(diffs);
}
