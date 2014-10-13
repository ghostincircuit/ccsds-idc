#include "dwt.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void dwt_all_rows(struct image *img, int scale, filter_t f)
{
        int i;
        unsigned char *dat = img->data;
        int step = img->cell_size;
        int cols = img->width;
        int rows = img->height;
        for (i = 0; i < rows/scale; i++) {
                f(dat + i*cols*step, cols/scale);
        }
}

static void dwt_all_cols(struct image *img, int scale, filter_t f)
{
        int i;
        u32 *dat = img->data;
        int cols = img->width;
        int rows = img->height;
        u32 *buf = malloc(rows*sizeof(img->cell_size));
        assert(buf);
        for (i = 0; i < cols/scale; i++) {
                int j;
                for (j = 0; j < rows/scale; j++)
                        buf[j] = dat[i + j*cols];
                f(buf, rows/scale);
                for (j = 0; j < rows/scale; j++)
                        dat[i + j*cols] = buf[j];
        }
        free(buf);
}

void dwt_2d(struct image *img, enum dwt_order order, int scale, filter_t f)
{
        assert(scale != 0);
        assert( ( (scale-1) & img->height ) == 0);
        assert( ( (scale-1) & img->width ) == 0);
        assert(f);
        assert(img);

        if (order == ROW_FIRST) {
                dwt_all_rows(img, scale, f);
                dwt_all_cols(img, scale, f);
        } else {
                dwt_all_cols(img, scale, f);
                dwt_all_rows(img, scale, f);
        }
}

static s32 lower(double i)
{
        if (i < 0) {
                double t = ((s32)i) - i;
                if (t)
                        return i;
                else
                        return i-1;
        }
        return i;
}

void dwt_53i(void *data, int len)
{
        int i;
        s32 *dat = data;
        int sz = len;
        int sz2 = len/2;
        s32 *ret = malloc(sizeof(dat[0]) * len);
        assert((sz&1) == 0);
        assert(ret);


        //high pass filter
        //and we must put high frequency coefficients on the right side
        for (i = 0; i < sz2; i++) {
                int i0 = 2*i;
                int i1 = 2*i+1;
                int i2;
                if (i == sz2-1)
                        i2 = 2*i;
                else
                        i2 = 2*i+2;
                s32 sum = lower(0.5*(dat[i0] + dat[i2]));
                sum = -sum + dat[i1];
                ret[sz2+i] = sum;
        }

        //low pass filter
        for (i = 0; i < sz2; i++) {
                int i0;
                int i1 = 2*i;
                int i2 = i;
                if (i == 0)
                        i0 = 0;
                else
                        i0 = i-1;
                int sum = lower(0.25*(ret[sz2+i0] + ret[sz2+i2]) + 0.5);
                sum = sum + dat[i1];
                ret[i] = sum;
        }
        memcpy(data, ret, len*sizeof(dat[0]));
        free(ret);
}

void idwt_53i(void *data, int len)
{
        int i;
        s32 *dat = data;
        int sz = len;
        int sz2 = len/2;
        s32 *ret = malloc(sizeof(dat[0]) * len);
        assert((sz&1) == 0);
        assert(ret);

        for (i = 0; i < sz2; i++) {
                int i0;
                if (i == 0)
                        i0 = 0;
                else
                        i0 = i-1;
                int i1 = i;
                int i2 = i;
                s32 sum = lower(0.25*(dat[sz2+i0] + dat[sz2+i2]) + 0.5);
                sum = -sum + dat[i1];
                ret[2*i] = sum;
        }

        for (i = 0; i < sz2; i++) {
                int i0 = 2*i;
                int i1 = i;
                int i2;
                if (i == sz2-1)
                        i2 = 2*i;
                else
                        i2 = 2*i+2;
                s32 sum = lower(0.5*(ret[i0] + ret[i2]));
                sum = sum + dat[sz2+i1];
                ret[2*i+1] = sum;
        }
        memcpy(data, ret, len*sizeof(dat[0]));
        free(ret);
}

void dwt_97i(void *data, int len)
{
        int i;
        s32 *dat = data;
        int sz = len;
        int sz2 = len/2;
        s32 *ret = malloc(sizeof(dat[0]) * len);
        assert((sz&1) == 0);
        assert(ret);

        for (i = 0; i < sz2; i++) {
                int i0 = 2*i-2;
                int i1 = 2*i;
                int i2 = 2*i+1;
                int i3 = 2*i+2;
                int i4 = 2*i+4;
                if (i == 0) {
                        i0 = 2;
                        i1 = 0;
                } else if (i == sz2-1) {
                        i3 = i1;
                        i4 = i0;
                } else if (i == sz2-2) {
                        i4 = i3;
                }
                s32 sum = dat[i2] - lower(9.0/16*(dat[i1] + dat[i3]) - 1.0/16*(dat[i0] + dat[i4]) + 0.5);
                ret[sz2+i] = sum;
        }

        for (i = 0; i < sz2; i++) {
                if (i == 0)
                        ret[i] = dat[0] - lower(-0.5*ret[0] + 0.5);
                else
                        ret[i] = dat[2*i] - lower(-0.25*(ret[sz2+i-1] + ret[sz2+i]) + 0.5);
        }

        memcpy(data, ret, len*sizeof(dat[0]));
        free(ret);
}

void idwt_97i(void *data, int len)
{
        int i;
        s32 *dat = data;
        int sz = len;
        int sz2 = len/2;
        s32 *ret = malloc(sizeof(dat[0]) * len);
        assert((sz&1) == 0);
        assert(ret);

        for (i = 0; i < sz2; i++) {
                if (i == 0)
                        ret[2*i] = dat[0] + lower(-0.5*dat[sz2+0] + 0.5);
                else
                        ret[2*i] = dat[i] + lower(-0.25*(dat[sz2+i-1] + dat[sz2+i]) + 0.5);
        }

        for (i = 0; i < sz2; i++) {
                int i0 = 2*i-2;
                int i1 = 2*i;
                int i2 = sz2+i;
                int i3 = 2*i+2;
                int i4 = 2*i+4;
                if (i == 0) {
                        i0 = 2;
                } else if (i == sz2-1) {
                        i3 = i1;
                        i4 = i0;
                } else if (i == sz2-2) {
                        i4 = i3;
                }
                ret[2*i+1] = dat[i2] + lower(9.0/16*(ret[i1] + ret[i3]) - 1.0/16*(ret[i0] + ret[i4]) + 0.5);
        }
        memcpy(data, ret, len*sizeof(dat[0]));
        free(ret);
}

static const double p97_low[] = {
        +0.852698679009,
        +0.377402855613,
        -0.110624404418,
        -0.023849465020,
        +0.037828455507
};

static const double p97_high[] = {
        -0.788485616406,
        +0.418092273222,
        +0.040689417609,
        -0.064538882629
};

void dwt_97f(void *data, int len)
{
        s32 *dat = data;
        int i,j;
        int sz = len;
        int sz2 = len/2;
        s32 *ret = malloc(sizeof(dat[0]) * len);
        assert((sz&1) == 0);
        assert(ret);

        for (i = 0; i < sz2; i++) {
                double ci = 0;
                //low pass filter
                for (j = -4; j <= 4; j++) {
                        int hi = j;
                        if (hi < 0) hi = -hi;
                        int dati = 2*i+j;
                        if (dati < 0)
                                dati = -dati;
                        else if (dati >= len)
                                dati = 2*(len-1)-dati;
                        ci += p97_low[hi] * dat[dati];
                }
                ret[i] = ci;

                double di = 0;
                //high pass filter
                for (j = -3; j <= 3; j++) {
                        int gi = j;
                        if (gi < 0) gi = -gi;
                        int dati = 2*i+1+j;
                        if (dati < 0)
                                dati = -dati;
                        else if (dati >= len)
                                dati = 2*(len-1)-dati;
                        di += p97_high[gi] * dat[dati];
                }
                ret[sz2+i] = di;
        }
        memcpy(data, ret, sizeof(dat[0])*len);
        free(ret);
}

void dwt_97ff(void *data, int len)
{
        float *dat = data;
        int i,j;
        int sz = len;
        int sz2 = len/2;
        float *ret = malloc(sizeof(dat[0]) * len);
        assert((sz&1) == 0);
        assert(ret);

        for (i = 0; i < sz2; i++) {
                double ci = 0;
                //low pass filter
                for (j = -4; j <= 4; j++) {
                        int hi = j;
                        if (hi < 0) hi = -hi;
                        int dati = 2*i+j;
                        if (dati < 0)
                                dati = -dati;
                        else if (dati >= len)
                                dati = 2*(len-1)-dati;
                        ci += p97_low[hi] * dat[dati];
                }
                ret[i] = ci;

                double di = 0;
                //high pass filter
                for (j = -3; j <= 3; j++) {
                        int gi = j;
                        if (gi < 0) gi = -gi;
                        int dati = 2*i+1+j;
                        if (dati < 0)
                                dati = -dati;
                        else if (dati >= len)
                                dati = 2*(len-1)-dati;
                        di += p97_high[gi] * dat[dati];
                }
                ret[sz2+i] = di;
        }
        memcpy(data, ret, sizeof(dat[0])*len);
        free(ret);
}

static const double pi97_high[] = {
        -0.852698679009,
        +0.377402855613,
        +0.110624404418,
        -0.023849465020,
        -0.037828455507
};

static const double pi97_low[] = {
        +0.788485616406,
        +0.418092273222,
        -0.040689417609,
        -0.064538882629
};

void idwt_97f(void *data, int len)
{
        s32 *dat = data;
        int i,j;
        int sz = len;
        int sz2 = len/2;
        s32 *ret = malloc(sizeof(dat[0]) * len);
        assert((sz&1) == 0);
        assert(ret);

        for (i = 0; i < sz2; i++) {
                //low pass synthesizer
                double even = 0;
                for (j = -1; j <= 1; j++) {
                        double term = 0;
                        int qi = 2*j;
                        if (qi < 0) qi = -qi;
                        int ci = i + j;
                        if (ci < 0)
                                ci = -ci;
                        else if (ci >= sz2)
                                ci = 2*sz2-1-ci;//watch this!!!! be careful!!! Not consistent
                        term = pi97_low[qi] * dat[ci];
                        even += term;
                }
                for (j = -2; j <= 1; j++) {
                        double term = 0;
                        int pi = 2*j+1;
                        if (pi < 0) pi = -pi;
                        int di = i + j;
                        if (di < 0)
                                di = -di-1;
                        else if (di >= sz2)
                                di = 2*(sz2-1)-di;
                        term = pi97_high[pi] * dat[sz2+di];
                        even += term;
                }
                ret[2*i] = even;

                //high pass synthesizer
                double odd = 0;
                for (j = -1; j <= 2; j++) {
                        double term = 0;
                        int qi = 2*j-1;
                        if (qi < 0) qi = -qi;
                        int ci = i+j;
                        if (ci < 0)
                                ci = -ci;
                        else if (ci >= sz2) {
                                ci = 2*sz2-1-ci;
                        }
                        term = pi97_low[qi] * dat[ci];
                        odd += term;
                }

                for (j = -2; j <= 2; j++) {
                        double term = 0;
                        int pi = 2*j;
                        if (pi < 0) pi = -pi;
                        int di = i+j;
                        if (di < 0)
                                di = -di-1;
                        else if (di >= sz2) {
                                di = 2*(sz2-1)-di;
                        }
                        term = pi97_high[pi] * dat[sz2+di];
                        odd += term;
                }
                ret[2*i+1] = odd;

        }
        memcpy(data, ret, sizeof(dat[0])*len);
        free(ret);
}

void idwt_97ff(void *data, int len)
{
        float *dat = data;
        int i,j;
        int sz = len;
        int sz2 = len/2;
        float *ret = malloc(sizeof(dat[0]) * len);
        assert((sz&1) == 0);
        assert(ret);

        for (i = 0; i < sz2; i++) {
                //low pass synthesizer
                double even = 0;
                for (j = -1; j <= 1; j++) {
                        double term = 0;
                        int qi = 2*j;
                        if (qi < 0) qi = -qi;
                        int ci = i + j;
                        if (ci < 0)
                                ci = -ci;
                        else if (ci >= sz2)
                                ci = 2*sz2-1-ci;//watch this!!!! be careful!!! Not consistent
                        term = pi97_low[qi] * dat[ci];
                        even += term;
                }
                for (j = -2; j <= 1; j++) {
                        double term = 0;
                        int pi = 2*j+1;
                        if (pi < 0) pi = -pi;
                        int di = i + j;
                        if (di < 0)
                                di = -di-1;
                        else if (di >= sz2)
                                di = 2*(sz2-1)-di;
                        term = pi97_high[pi] * dat[sz2+di];
                        even += term;
                }
                ret[2*i] = even;

                //high pass synthesizer
                double odd = 0;
                for (j = -1; j <= 2; j++) {
                        double term = 0;
                        int qi = 2*j-1;
                        if (qi < 0) qi = -qi;
                        int ci = i+j;
                        if (ci < 0)
                                ci = -ci;
                        else if (ci >= sz2) {
                                ci = 2*sz2-1-ci;
                        }
                        term = pi97_low[qi] * dat[ci];
                        odd += term;
                }

                for (j = -2; j <= 2; j++) {
                        double term = 0;
                        int pi = 2*j;
                        if (pi < 0) pi = -pi;
                        int di = i+j;
                        if (di < 0)
                                di = -di-1;
                        else if (di >= sz2) {
                                di = 2*(sz2-1)-di;
                        }
                        term = pi97_high[pi] * dat[sz2+di];
                        odd += term;
                }
                ret[2*i+1] = odd;

        }
        memcpy(data, ret, sizeof(dat[0])*len);
        free(ret);
}

#define _UNIT_TEST_DWT_
#ifdef _UNIT_TEST_DWT_
int main()
{
        const int N = 32;
        const int N2 = 2028;
        s32 a[N2];
        float ff[N];
        int f[N];
        int i;
        for (i = 0; i < N2; i++)
                a[i] = i*i;
        dwt_53i(a, N2);
        idwt_53i(a, N2);
        for (i = 0; i < N2; i++)
                assert(a[i] == i*i);

        for (i = 0; i < N; i++)
                a[i] = i;
        dwt_97i(a, N);
        idwt_97i(a, N);
        for (i = 0; i< N; i++)
                assert(a[i] == i);

        for (i = 0; i < N; i++) {
                ff[i] = (float)i;
                f[i] = i;
        }

        dwt_97ff(ff, N);
        idwt_97ff(ff, N);

        dwt_97f(f, N);
        idwt_97f(f, N);

        //const char rei[] = "../res/1024_768_8_1_l_rei_with_penpen.raw";
        const char rei[] = "../res/800_600_8_1_l_asuka_beach.raw";
        struct image *org = image_loader_create(rei);
        struct image *img = image_loader_copy(org);

        //transform
        dwt_2d(img, ROW_FIRST, 1, dwt_53i);
        dwt_2d(img, ROW_FIRST, 2, dwt_53i);
        dwt_2d(img, ROW_FIRST, 4, dwt_53i);

        //save tranformed image
        image_loader_save(img, "trans.raw", "./", 0);
/*
        image_loader_log(org);
        printf("========================================================\n");
        dwt_2d(img, COL_FIRST, 1, idwt_53i);
        image_loader_log(img);
*/

        //invert tranformation
        dwt_2d(img, COL_FIRST, 4, idwt_53i);
        dwt_2d(img, COL_FIRST, 2, idwt_53i);
        dwt_2d(img, COL_FIRST, 1, idwt_53i);
        image_loader_assert_equal(org, img);

        //save invert transformed
        s32 *dat = img->data;
        for (i = 0; i < img->height * img->width; i++)
                if (dat[i] < 0)
                        dat[i] = -dat[i];
        image_loader_save(img, "inv.raw", "./", 0);
        image_loader_free(img);
        image_loader_free(org);
        return 0;
}
#endif//_UNIT_TEST_DWT_
