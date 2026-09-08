#ifndef WATCH_POWER_H
#define WATCH_POWER_H

#include <stdbool.h>
#include <stdint.h>

#define WATCH_POWER_LOW_BATTERY_PERCENT 15
#define WATCH_POWER_SAMPLE_PERIOD_MS 5000

typedef struct
{
  int battery_mv;
  int battery_percent;
  bool vbus_present;
  bool charging;
  bool valid;
} watch_power_snapshot_t;

int watch_power_init(void);
void watch_power_poll(uint32_t now_ms);
void watch_power_get(watch_power_snapshot_t *snapshot);
bool watch_power_low_battery_event(void);

#endif
