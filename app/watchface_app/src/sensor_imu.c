/****************************************************************************
 * sensor_imu.c - LSM6DSL/LSM6DS3TR-C integration for the watchface app
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <nuttx/sensors/lsm6dsl.h>

#include "sensor_imu.h"

/* The board driver configures the accelerometer for +-16 g. */
#define MG_PER_LSB_NUM       488u
#define MG_PER_LSB_DEN       1000u

/* Pedometer tuning: dynamic swing in mg above/below 1 g gravity. */
#define STEP_ARM_MG          180
#define STEP_DISARM_MG       90
#define STEP_MIN_GAP_MS      250
#define STEP_MAX_GAP_MS      1400

/* Wrist raise is deliberately conservative and edge-triggered. */
#define WRIST_RAISE_DELTA_MG       500
#define WRIST_RAISE_MIN_MAG_MG     650
#define WRIST_RAISE_MAX_MAG_MG     1400
#define WRIST_RAISE_COOLDOWN_MS    1200

#define IMU_DEVICE_PATH      "/dev/lsm6dsl0"
#define STEPS_FILE_PATH      "/data/watch_steps.dat"
#define STEPS_TEMP_PATH      "/data/watch_steps.dat.tmp"
#define STEPS_MAGIC           0x57535450u /* "WSTP" */
#define STEPS_VERSION         1u

struct steps_record_s
{
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  uint32_t steps;
  uint32_t crc;
};

static int g_fd = -1;
static int g_mag_mg = 1000;
static int g_dynamic_mg;
static int g_energy;
static uint32_t g_steps;
static bool g_steps_loaded;
static bool g_step_armed;
static uint32_t g_last_step_ms;
static bool g_wrist_event;
static bool g_have_previous_accel;
static int g_previous_mx;
static int g_previous_my;
static int g_previous_mz;
static uint32_t g_last_wrist_ms;

static uint32_t imu_now_ms(void)
{
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint32_t)ts.tv_sec * 1000u +
         (uint32_t)(ts.tv_nsec / 1000000u);
}

static uint32_t steps_crc(const struct steps_record_s *record)
{
  const uint8_t *data = (const uint8_t *)record;
  size_t length = offsetof(struct steps_record_s, crc);
  uint32_t crc = 0xffffffffu;

  while (length-- != 0)
    {
      crc ^= *data++;
      for (int bit = 0; bit < 8; bit++)
        {
          crc = (crc >> 1) ^ (0xedb88320u &
                (uint32_t)-(int32_t)(crc & 1u));
        }
    }

  return ~crc;
}

static bool read_full(int fd, void *buffer, size_t length)
{
  uint8_t *cursor = (uint8_t *)buffer;

  while (length != 0)
    {
      ssize_t received = read(fd, cursor, length);
      if (received <= 0)
        {
          return false;
        }
      cursor += received;
      length -= (size_t)received;
    }

  return true;
}

static bool write_full(int fd, const void *buffer, size_t length)
{
  const uint8_t *cursor = (const uint8_t *)buffer;

  while (length != 0)
    {
      ssize_t written = write(fd, cursor, length);
      if (written <= 0)
        {
          return false;
        }
      cursor += written;
      length -= (size_t)written;
    }

  return true;
}

int imu_steps_load(void)
{
  struct steps_record_s record;
  int fd;
  bool valid;

  if (g_steps_loaded)
    {
      return 0;
    }

  g_steps_loaded = true;
  g_steps = 0;
  fd = open(STEPS_FILE_PATH, O_RDONLY);
  if (fd < 0)
    {
      return -1;
    }

  valid = read_full(fd, &record, sizeof(record));
  (void)close(fd);
  if (!valid || record.magic != STEPS_MAGIC ||
      record.version != STEPS_VERSION ||
      record.crc != steps_crc(&record))
    {
      return -1;
    }

  g_steps = record.steps;
  return 0;
}

int imu_steps_save(void)
{
  struct steps_record_s record;
  int fd;
  int ret;

  (void)mkdir("/data", 0755);
  record.magic = STEPS_MAGIC;
  record.version = STEPS_VERSION;
  record.reserved = 0;
  record.steps = g_steps;
  record.crc = steps_crc(&record);

  fd = open(STEPS_TEMP_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0)
    {
      return -1;
    }

  if (!write_full(fd, &record, sizeof(record)))
    {
      (void)close(fd);
      (void)unlink(STEPS_TEMP_PATH);
      return -1;
    }

  (void)fsync(fd);
  ret = close(fd);
  if (ret == 0 && rename(STEPS_TEMP_PATH, STEPS_FILE_PATH) == 0)
    {
      return 0;
    }

  (void)unlink(STEPS_TEMP_PATH);
  return -1;
}

int imu_init(void)
{
  struct lsm6dsl_sensor_data_s data;

  (void)imu_steps_load();
  g_step_armed = false;
  g_have_previous_accel = false;
  g_wrist_event = false;

  g_fd = open(IMU_DEVICE_PATH, O_RDONLY);
  if (g_fd < 0)
    {
      printf("imu: open %s failed\n", IMU_DEVICE_PATH);
      return -1;
    }

  if (ioctl(g_fd, SNIOC_START, 0) < 0)
    {
      printf("imu: SNIOC_START failed\n");
    }

  if (ioctl(g_fd, SNIOC_LSM6DSLSENSORREAD,
            (unsigned long)&data) == 0)
    {
      printf("imu: ok raw x=%d y=%d z=%d\n",
             data.x_data, data.y_data, data.z_data);
    }

  return 0;
}

void imu_poll(void)
{
  struct lsm6dsl_sensor_data_s data;
  int mx;
  int my;
  int mz;
  uint32_t now;

  if (g_fd < 0 || ioctl(g_fd, SNIOC_LSM6DSLSENSORREAD,
                       (unsigned long)&data) != 0)
    {
      return;
    }

  mx = (int)data.x_data * MG_PER_LSB_NUM / MG_PER_LSB_DEN;
  my = (int)data.y_data * MG_PER_LSB_NUM / MG_PER_LSB_DEN;
  mz = (int)data.z_data * MG_PER_LSB_NUM / MG_PER_LSB_DEN;
  g_mag_mg = (int)sqrtf((float)mx * mx + (float)my * my +
                        (float)mz * mz);
  g_dynamic_mg = g_mag_mg - 1000;
  if (g_dynamic_mg < 0)
    {
      g_dynamic_mg = -g_dynamic_mg;
    }

  now = imu_now_ms();

  /* Arm on a strong peak and count when the signal returns to baseline. */
  if (!g_step_armed)
    {
      if (g_dynamic_mg >= STEP_ARM_MG)
        {
          g_step_armed = true;
        }
    }
  else if (g_dynamic_mg <= STEP_DISARM_MG)
    {
      uint32_t gap = now - g_last_step_ms;
      if (g_last_step_ms == 0 ||
          (gap >= STEP_MIN_GAP_MS && gap <= STEP_MAX_GAP_MS))
        {
          g_steps++;
          (void)imu_steps_save();
          g_last_step_ms = now;
        }
      g_step_armed = false;
    }

  /* A large gravity-vector change with a normal final magnitude is a lift. */
  if (g_have_previous_accel)
    {
      int delta = abs(mx - g_previous_mx) +
                  abs(my - g_previous_my) + abs(mz - g_previous_mz);
      if (delta >= WRIST_RAISE_DELTA_MG &&
          g_mag_mg >= WRIST_RAISE_MIN_MAG_MG &&
          g_mag_mg <= WRIST_RAISE_MAX_MAG_MG &&
          (g_last_wrist_ms == 0 ||
           now - g_last_wrist_ms >= WRIST_RAISE_COOLDOWN_MS))
        {
          g_wrist_event = true;
          g_last_wrist_ms = now;
        }
    }
  g_previous_mx = mx;
  g_previous_my = my;
  g_previous_mz = mz;
  g_have_previous_accel = true;

  /* Smoothed motion energy for the stress/activity page (0..100). */
  {
    int target = g_dynamic_mg / 4;
    if (target > 100)
      {
        target = 100;
      }

    if (target > g_energy)
      {
        g_energy += (target - g_energy + 1) / 2;
      }
    else
      {
        g_energy -= (g_energy - target + 7) / 8;
      }
  }
}

long imu_steps(void)
{
  return (long)g_steps;
}

int imu_motion_energy(void)
{
  return g_energy;
}

bool imu_wrist_raise(void)
{
  bool raised = g_wrist_event;

  g_wrist_event = false;
  return raised;
}
