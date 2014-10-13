#pragma once

#include "utils.h"
#include "image_loader.h"

enum dwt_order {COLUMN_FIRST, ROW_FIRST};

typedef void (*filter_t)(void *data, int len);


//scale: 1 means all, 2 means left corner 1/4, 3 menas left corner 1/16
//this can do both transform and invert transform
void dwt2d(struct image *img, enum dwt_order order, int scale, filter_t f);

void dwt_53i(u32 data[], int len);
void dwt_97i(u32 data[], int len);
void dwt_97f(float data[], int len);

void idwt_53i(u32 data[], int len);
void idwt_97i(u32 data[], int len);
void idwt_97f(float data[], int len);
