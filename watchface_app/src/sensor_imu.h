#ifndef WATCH_SENSOR_IMU_H
#define WATCH_SENSOR_IMU_H

#include <stdbool.h>

int imu_init(void);
void imu_poll(void);
long imu_steps(void);
int imu_motion_energy(void);
int imu_steps_load(void);
int imu_steps_save(void);
bool imu_wrist_raise(void);

#endif
