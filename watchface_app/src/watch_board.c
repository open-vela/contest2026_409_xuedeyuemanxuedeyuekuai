#include <stdbool.h>

#include "watch_board.h"

/* The Huangshan board supplies the strong implementation in its lower-half.
 * These weak defaults keep the application portable to a board without VBUS
 * sensing or a dedicated vibrator pin. */
__attribute__((weak)) void watch_board_init(void)
{
}

__attribute__((weak)) bool watch_board_vbus_present(void)
{
  return false;
}
