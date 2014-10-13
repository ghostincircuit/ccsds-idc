#pragma once

#include "utils.h"
#include "image_loader.h"

enum dwt_order {COL_FIRST, ROW_FIRST};

typedef void (*filter_t)(void *data, int len);


//scale: 1 means all, 2 means left corner 1/4, 3 menas left corner 1/16
//this can do both transform and invert transform
void dwt_2d(struct image *img, enum dwt_order order, int scale, filter_t f);

void dwt_53i(void *data, int len);
void idwt_53i(void *data, int len);

void dwt_97i(void *data, int len);
void idwt_97i(void *data, int len);

void dwt_97f(void *data, int len);
void idwt_97f(void *data, int len);

void dwt_97ff(void *data, int len);
void idwt_97ff(void *data, int len);
