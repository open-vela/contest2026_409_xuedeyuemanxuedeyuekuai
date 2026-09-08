#include <nuttx/config.h>

#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>

#include "watch_board.h"
#include "watch_power.h"

#define WATCH_POWER_ADC_PATH "/dev/adc0"
#define ADC_CHAN_VBAT 5 /* SF32LB52 internal VBAT channel */
#define WATCH_POWER_EMPTY_MV 3300
#define WATCH_POWER_FULL_MV  4200
#define WATCH_POWER_VBUS_POLL_PERIOD_MS 500

static int g_adc_fd = -1;
static pthread_mutex_t g_power_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t g_power_thread;
static bool g_power_thread_started;
static uint32_t g_last_sample_ms;
static uint32_t g_last_vbus_ms;
static bool g_low_battery_pending;
static bool g_low_battery_latched;
static watch_power_snapshot_t g_snapshot =
{
  .battery_mv = WATCH_POWER_FULL_MV,
  .battery_percent = 100,
  .vbus_present = false,
  .charging = false,
  .valid = false,
};

static int battery_percent_from_mv(int battery_mv)
{
  int battery_percent = (battery_mv - WATCH_POWER_EMPTY_MV) * 100 /
                        (WATCH_POWER_FULL_MV - WATCH_POWER_EMPTY_MV);

  if (battery_percent < 0)
    {
      battery_percent = 0;
    }
  if (battery_percent > 100)
    {
      battery_percent = 100;
    }
  return battery_percent;
}

static void power_update_vbus(bool vbus)
{
  pthread_mutex_lock(&g_power_lock);
  g_snapshot.vbus_present = vbus;
  g_snapshot.charging = vbus;
  pthread_mutex_unlock(&g_power_lock);
}

static void power_sample(void)
{
  struct adc_msg_s sample;
  watch_power_snapshot_t next;
  bool vbus;
  ssize_t received;

  vbus = watch_board_vbus_present();
  if (ioctl(g_adc_fd, ANIOC_TRIGGER, 0) < 0)
    {
      power_update_vbus(vbus);
      return;
    }

  memset(&sample, 0, sizeof(sample));
  received = read(g_adc_fd, &sample, sizeof(sample));
  if (received != (ssize_t)sizeof(sample) ||
      sample.am_channel != ADC_CHAN_VBAT)
    {
      power_update_vbus(vbus);
      return;
    }

  next.battery_mv = sample.am_data;
  next.battery_percent = battery_percent_from_mv(sample.am_data);
  next.vbus_present = vbus;
  next.charging = vbus;
  next.valid = true;

  pthread_mutex_lock(&g_power_lock);
  g_snapshot = next;
  if (next.battery_percent <= WATCH_POWER_LOW_BATTERY_PERCENT)
    {
      if (!g_low_battery_latched)
        {
          g_low_battery_pending = true;
          g_low_battery_latched = true;
        }
    }
  else if (next.battery_percent > WATCH_POWER_LOW_BATTERY_PERCENT + 5)
    {
      g_low_battery_latched = false;
    }
  pthread_mutex_unlock(&g_power_lock);
}

static void *power_worker(void *arg)
{
  (void)arg;

  while (true)
    {
      power_sample();
      usleep(WATCH_POWER_SAMPLE_PERIOD_MS * 1000u);
    }

  return NULL;
}

int watch_power_init(void)
{
  int ret;

  g_adc_fd = open(WATCH_POWER_ADC_PATH, O_RDONLY | O_NONBLOCK);
  if (g_adc_fd < 0)
    {
      printf("power: open %s failed\n", WATCH_POWER_ADC_PATH);
      return -1;
    }

  power_update_vbus(watch_board_vbus_present());
  g_last_sample_ms = 0;
  g_last_vbus_ms = 0;
  ret = pthread_create(&g_power_thread, NULL, power_worker, NULL);
  if (ret == 0)
    {
      pthread_detach(g_power_thread);
      g_power_thread_started = true;
    }
  else
    {
      printf("power: worker start failed: %d\n", ret);
      g_power_thread_started = false;
    }

  return 0;
}

void watch_power_poll(uint32_t now_ms)
{
  bool vbus;

  if (g_adc_fd < 0)
    {
      return;
    }

  if (g_last_vbus_ms == 0 ||
      now_ms - g_last_vbus_ms >= WATCH_POWER_VBUS_POLL_PERIOD_MS)
    {
      g_last_vbus_ms = now_ms;
      vbus = watch_board_vbus_present();
      power_update_vbus(vbus);
    }

  /* A failed pthread setup still has a functional, low-rate fallback. */
  if (!g_power_thread_started &&
      (g_last_sample_ms == 0 ||
       now_ms - g_last_sample_ms >= WATCH_POWER_SAMPLE_PERIOD_MS))
    {
      g_last_sample_ms = now_ms;
      power_sample();
    }
}

void watch_power_get(watch_power_snapshot_t *snapshot)
{
  if (snapshot != NULL)
    {
      pthread_mutex_lock(&g_power_lock);
      *snapshot = g_snapshot;
      pthread_mutex_unlock(&g_power_lock);
    }
}

bool watch_power_low_battery_event(void)
{
  bool pending;

  pthread_mutex_lock(&g_power_lock);
  pending = g_low_battery_pending;
  g_low_battery_pending = false;
  pthread_mutex_unlock(&g_power_lock);
  return pending;
}
