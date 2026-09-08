#ifndef __WATCHFACE_HONEYCOMB_MODEL_H
#define __WATCHFACE_HONEYCOMB_MODEL_H

#include <stdint.h>

typedef struct
{
  int8_t q;
  int8_t r;
} honeycomb_cell_t;

typedef struct
{
  int32_t x;
  int32_t y;
} honeycomb_point_t;

void honeycomb_cell_point(const honeycomb_cell_t *cell,
                          int32_t spacing_x,
                          int32_t spacing_y,
                          honeycomb_point_t *point);

int honeycomb_nearest_cell(const honeycomb_cell_t *cells,
                           int count,
                           int32_t offset_x,
                           int32_t offset_y,
                           int32_t spacing_x,
                           int32_t spacing_y);

void honeycomb_snap_offset(const honeycomb_cell_t *cell,
                           int32_t spacing_x,
                           int32_t spacing_y,
                           int32_t *offset_x,
                           int32_t *offset_y);

int32_t honeycomb_clamp(int32_t value, int32_t minimum, int32_t maximum);

#endif
