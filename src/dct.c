#include <stdlib.h>
#include <math.h>
#include <string.h>

static const double inv2 = 0.7071067811865476;
static const double pi = 3.14159265359;

void dct(double dv[], int n)
{

        double *ret = malloc(sizeof(double)*n);
        double C = sqrt(2.0/n);
        int i;
        for (i = 0; i < n; i++) {
                double sum = 0;
                int j;
                for (j = 0; j < n; j++) {
                        double a;
                        if (i == 0)
                                a = inv2;
                        else
                                a = 1;
                        double term = a * cos(pi*i*(2*j+1.0)/(2.0*n)) * dv[j];
                        sum += term;
                }
                ret[i] = sum * C;;
        }
        memcpy(dv, ret, sizeof(double)*n);
}

void idct(double dv[], int n)
{
        double *ret = malloc(sizeof(double)*n);
        double C = sqrt(2.0/n);
        int i;
        for (i = 0; i < n; i++) {
                double sum = 0;
                double a;
                int j;
                for (j = 0; j < n; j++) {
                        double a;
                        if (j == 0)
                                a = inv2;
                        else
                                a = 1;
                        double term = a * cos(pi*j*(2*i+1.0)/(2.0*n)) * dv[j];
                        sum += term;
                }
                ret[i] = sum * C;;
        }
        memcpy(dv, ret, sizeof(double)*n);
}

#define _TEST_DCT_
#ifdef _TEST_DCT_
#include <stdio.h>
int main()
{
        int i;
        double data[] = {8,0,8,0,8,0,8,0};
        dct(data, 8);
        for (i = 0; i < 8; i++)
                printf("%f ", data[i]);
        printf("\n");
        idct(data, 8);
        for (i = 0; i < 8; i++)
                printf("%f ", data[i]);
        printf("\n");
        return 0;
}
#endif
