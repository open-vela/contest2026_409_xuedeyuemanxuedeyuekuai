/****************************************************************************
 * buttons.c - Button handling for HuangshanPi watchface
 *
 * KEY1: Short press - Back / wake the previous screen
 *       Long press  - Open the system operation screen
 * KEY2: Short press - Open the application menu
 *
 * The board lower-half owns the emergency reset gesture.  This application
 * layer only reports user-interface gestures and never resets the system.
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "lvgl.h"

#define BUTTON_KEY1 0
#define BUTTON_KEY2 1

#define KEY1_BIT 0x01u
#define KEY2_BIT 0x02u

#define POLL_MS 20
#define DEBOUNCE_MS 30
#define LONG_PRESS_MS 2000

#define PRESS_SHORT 0
#define PRESS_LONG 1

typedef void (*button_cb_t)(int button, int press_type);

static lv_timer_t *g_btn_timer;
static button_cb_t g_button_cb;
static int g_btn_fd = -1;
static bool g_pressed[2];
static bool g_long_sent[2];
static uint32_t g_press_tick[2];

static void button_emit(int button, int press_type)
{
  if (g_button_cb != NULL)
    {
      g_button_cb(button, press_type);
    }
}

static void button_update_state(uint32_t sample, uint32_t now)
{
  static const uint32_t bits[2] = { KEY1_BIT, KEY2_BIT };

  for (int i = 0; i < 2; i++)
    {
      bool down = (sample & bits[i]) != 0;

      if (down != g_pressed[i])
        {
          if (down)
            {
              g_pressed[i] = true;
              g_long_sent[i] = false;
              g_press_tick[i] = now;
            }
          else
            {
              uint32_t held = now - g_press_tick[i];
              g_pressed[i] = false;

              if (!g_long_sent[i] && held >= DEBOUNCE_MS)
                {
                  button_emit(i, PRESS_SHORT);
                }
            }
        }

      /* Keep hold detection independent of new driver read events. */
      if (g_pressed[i] && !g_long_sent[i] &&
          now - g_press_tick[i] >= LONG_PRESS_MS)
        {
          g_long_sent[i] = true;
          button_emit(i, PRESS_LONG);
        }
    }
}

static void btn_timer_cb(lv_timer_t *timer)
{
  uint32_t sample;
  uint32_t now;
  ssize_t received;

  LV_UNUSED(timer);

  if (g_btn_fd < 0)
    {
      return;
    }

  received = read(g_btn_fd, &sample, sizeof(sample));
  now = lv_tick_get();

  if (received == (ssize_t)sizeof(sample))
    {
      button_update_state(sample, now);
    }
  else if (received < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
      /* Do not invent a release on EAGAIN; the driver is event based. */
      g_pressed[BUTTON_KEY1] = false;
      g_pressed[BUTTON_KEY2] = false;
      g_long_sent[BUTTON_KEY1] = false;
      g_long_sent[BUTTON_KEY2] = false;
    }
  else
    {
      /* No new edge is still a valid sample for long-press timing. */
      for (int i = 0; i < 2; i++)
        {
          if (g_pressed[i] && !g_long_sent[i] &&
              now - g_press_tick[i] >= LONG_PRESS_MS)
            {
              g_long_sent[i] = true;
              button_emit(i, PRESS_LONG);
            }
        }
    }
}

int buttons_init(button_cb_t cb)
{
  g_button_cb = cb;
  g_btn_fd = open("/dev/buttons", O_RDONLY | O_NONBLOCK);
  if (g_btn_fd < 0)
    {
      printf("buttons: open /dev/buttons failed\n");
      return -1;
    }

  g_btn_timer = lv_timer_create(btn_timer_cb, POLL_MS, NULL);
  if (g_btn_timer == NULL)
    {
      close(g_btn_fd);
      g_btn_fd = -1;
      return -1;
    }

  printf("buttons: /dev/buttons ready, long press=%ums\n",
         (unsigned int)LONG_PRESS_MS);
  return 0;
}

void buttons_deinit(void)
{
  if (g_btn_timer != NULL)
    {
      lv_timer_del(g_btn_timer);
      g_btn_timer = NULL;
    }

  if (g_btn_fd >= 0)
    {
      close(g_btn_fd);
      g_btn_fd = -1;
    }

  g_button_cb = NULL;
  g_pressed[BUTTON_KEY1] = false;
  g_pressed[BUTTON_KEY2] = false;
  g_long_sent[BUTTON_KEY1] = false;
  g_long_sent[BUTTON_KEY2] = false;
}

void buttons_simulate(int button, int press_type)
{
  printf("buttons: simulate button=%d press=%d\n", button, press_type);
  button_emit(button, press_type);
}
