#include <nuttx/config.h>

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include <nuttx/timers/pwm.h>

#include "watch_haptics.h"

#define WATCH_HAPTICS_PWM_PATH "/dev/pwm0"
#define WATCH_HAPTICS_FREQUENCY 220
#define WATCH_HAPTICS_DUTY 32768

static int g_pwm_fd = -1;
static bool g_enabled = true;
static bool g_running;
static uint32_t g_stop_ms;

static uint32_t haptics_now_ms(void)
{
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint32_t)ts.tv_sec * 1000u +
         (uint32_t)(ts.tv_nsec / 1000000u);
}

int watch_haptics_init(void)
{
  g_pwm_fd = open(WATCH_HAPTICS_PWM_PATH, O_RDWR | O_NONBLOCK);
  if (g_pwm_fd < 0)
    {
      printf("haptics: open %s failed\n", WATCH_HAPTICS_PWM_PATH);
      return -1;
    }
  return 0;
}

void watch_haptics_set_enabled(bool enabled)
{
  g_enabled = enabled;
  if (!enabled && g_running)
    {
      ioctl(g_pwm_fd, PWMIOC_STOP, 0);
      g_running = false;
    }
}

bool watch_haptics_get_enabled(void)
{
  return g_enabled;
}

void watch_haptics_pulse(uint32_t duration_ms)
{
  struct pwm_info_s info;

  if (!g_enabled || g_pwm_fd < 0 || duration_ms == 0)
    {
      return;
    }

  info.frequency = WATCH_HAPTICS_FREQUENCY;
  info.duty = WATCH_HAPTICS_DUTY;
  info.cpol = PWM_CPOL_HIGH;
  info.dcpol = PWM_DCPOL_LOW;
  if (ioctl(g_pwm_fd, PWMIOC_SETCHARACTERISTICS,
            (unsigned long)&info) < 0 ||
      ioctl(g_pwm_fd, PWMIOC_START, 0) < 0)
    {
      return;
    }

  g_running = true;
  g_stop_ms = haptics_now_ms() + duration_ms;
}

void watch_haptics_poll(uint32_t now_ms)
{
  if (g_running && (int32_t)(now_ms - g_stop_ms) >= 0)
    {
      ioctl(g_pwm_fd, PWMIOC_STOP, 0);
      g_running = false;
    }
}
