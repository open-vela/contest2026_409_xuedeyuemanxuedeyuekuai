/****************************************************************************
 * honeycomb_model.c - Pure geometry for the watch menu.
 ****************************************************************************/

#include <stddef.h>

#include "honeycomb_model.h"

static int64_t distance_squared(const honeycomb_cell_t *cell,
                                int32_t offset_x,
                                int32_t offset_y,
                                int32_t spacing_x,
                                int32_t spacing_y)
{
  honeycomb_point_t point;
  int64_t x;
  int64_t y;

  honeycomb_cell_point(cell, spacing_x, spacing_y, &point);
  x = (int64_t)point.x + offset_x;
  y = (int64_t)point.y + offset_y;
  return x * x + y * y;
}

void honeycomb_cell_point(const honeycomb_cell_t *cell,
                          int32_t spacing_x,
                          int32_t spacing_y,
                          honeycomb_point_t *point)
{
  if (cell == NULL || point == NULL)
    {
      return;
    }

  /* Axial coordinates produce a regular hexagonal lattice. */
  point->x = (int32_t)cell->q * spacing_x +
             ((int32_t)cell->r * spacing_x) / 2;
  point->y = (int32_t)cell->r * spacing_y;
}

int honeycomb_nearest_cell(const honeycomb_cell_t *cells,
                           int count,
                           int32_t offset_x,
                           int32_t offset_y,
                           int32_t spacing_x,
                           int32_t spacing_y)
{
  int best = -1;
  int64_t best_distance = 0;

  if (cells == NULL || count <= 0)
    {
      return -1;
    }

  for (int i = 0; i < count; i++)
    {
      int64_t distance = distance_squared(&cells[i], offset_x, offset_y,
                                          spacing_x, spacing_y);
      if (best < 0 || distance < best_distance)
        {
          best = i;
          best_distance = distance;
        }
    }

  return best;
}

void honeycomb_snap_offset(const honeycomb_cell_t *cell,
                           int32_t spacing_x,
                           int32_t spacing_y,
                           int32_t *offset_x,
                           int32_t *offset_y)
{
  honeycomb_point_t point;

  if (offset_x == NULL || offset_y == NULL)
    {
      return;
    }

  honeycomb_cell_point(cell, spacing_x, spacing_y, &point);
  *offset_x = -point.x;
  *offset_y = -point.y;
}

int32_t honeycomb_clamp(int32_t value, int32_t minimum, int32_t maximum)
{
  if (minimum > maximum)
    {
      int32_t temporary = minimum;
      minimum = maximum;
      maximum = temporary;
    }

  if (value < minimum)
    {
      return minimum;
    }

  if (value > maximum)
    {
      return maximum;
    }

  return value;
}
