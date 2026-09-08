#ifndef WATCH_HAPTICS_H
#define WATCH_HAPTICS_H

#include <stdbool.h>
#include <stdint.h>

int watch_haptics_init(void);
void watch_haptics_set_enabled(bool enabled);
bool watch_haptics_get_enabled(void);
void watch_haptics_pulse(uint32_t duration_ms);
void watch_haptics_poll(uint32_t now_ms);

#endif
