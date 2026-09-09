
/**************************************************************************** * apps/examples/watchface/src/watchface_main.c * * HuangshanPi (SF32LB52) openvela multi-face watch UI. ****************************************************************************/
#include <nuttx/config.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <nuttx/lcd/lcd_dev.h>
#include "lvgl.h"
#include "wf_assets.h"
#include "honeycomb_model.h"
#include "sensor_imu.h"
#include "watch_board.h"
#include "watch_haptics.h"
#include "watch_power.h"
extern const lv_font_t font_zh_16;
extern const lv_font_t font_zh_20;
/* Forward declarations */
static void build_health_enhanced(void);
static void build_game_enhanced(void);
static void update_health_display(void);
static void init_particles(void);
static void spawn_hit_particles(int32_t x, int32_t y, lv_color_t color);
#define NFACES           4
#define FACE_DIAL        0
#define FACE_MICKEY      1
#define FACE_DIGITAL     2
#define FACE_SPORT       3
#define MODE_WATCHFACE   0
#define MODE_HEALTH      1
#define MODE_GAME        2
#define MODE_MENU        3
#define MODE_SELECTOR    4
#define MODE_SYSTEM      5
#define MODE_APP         6
#define PRESS_SHORT       0
#define PRESS_LONG        1
#define GAME_NOTES_MAX   8
#define GAME_SPAWN_MS    1200
#define GAME_SHRINK_MS   2400
#define GAME_NOTE_R      18
#define GAME_TARGET_R    60
#define GAME_HIT_ZONE    30
#define FACE_STATS_REPORT_MS 5000
#define FACE_TICK_PERIOD_MS 33
#define HONEY_MENU_ANIM_PERIOD_MS 16
#define APP_RENDER_PERIOD_MS 33
#define SENSOR_POLL_PERIOD_MS 200
#define WATCHFACE_IDLE_TIMEOUT_MS 30000
#define FACE_DESIGN_SIZE 480
typedef struct {
  lv_obj_t *scr;
  lv_obj_t *bg;
  lv_obj_t *hour, *min, *sec;
  lv_obj_t *mickey_head, *mickey_body, *mickey_eye;
  lv_obj_t *mickey_hour, *mickey_minute;
  lv_obj_t *sport_hour, *sport_minute, *sport_second;
  lv_obj_t *digits[4];
  lv_obj_t *lbl_hm, *lbl_ss, *lbl_date, *lbl_secs, *lbl_week, *lbl_colon;
  lv_obj_t *lbl_card[3];
  lv_obj_t *face_arc[3];
  lv_obj_t *lbl_batt, *batt_fill;
  lv_obj_t *eye, *lbl_time;
  lv_obj_t *lbl_steps, *lbl_cal, *lbl_dist, *lbl_sport_hr;} face_ui_t;
typedef struct {
  lv_obj_t *scr, *lbl_time, *lbl_date;
  lv_obj_t *lbl_hr, *arc_hr;
  lv_obj_t *lbl_spo2, *arc_spo2;
  lv_obj_t *lbl_stress, *arc_stress;
  lv_obj_t *lbl_steps, *lbl_battery, *arc_battery;
  lv_obj_t *lbl_power_status;} health_ui_t;
typedef struct {
  lv_obj_t *obj;  int32_t cx, cy;  uint32_t spawn_tick;  bool active;} game_note_t;
typedef struct {
  lv_obj_t *scr, *target_ring, *lbl_score, *lbl_combo, *lbl_judge;  game_note_t notes[GAME_NOTES_MAX];  uint32_t last_spawn, score, combo, max_combo;  bool running;} game_ui_t;
static lv_display_t *g_disp;
static int g_w, g_h;
static face_ui_t g_face[NFACES];
static lv_obj_t *g_selector;
static bool g_selecting;
static int g_selector_face;
static lv_obj_t *g_selector_preview;
static lv_obj_t *g_selector_preview_left;
static lv_obj_t *g_selector_preview_right;
static lv_obj_t *g_selector_touch;
static lv_obj_t *g_selector_title;
static bool g_selector_dragging;
static bool g_selector_moved;
static lv_point_t g_selector_last_point;
static int g_current;
static struct timespec g_boot_mono;
static time_t g_fake_base = -1;
static int g_mode = MODE_WATCHFACE;
static uint32_t g_face_tick_count;
static uint32_t g_face_tick_elapsed_ms;
static uint32_t g_face_stats_last_report;
#define HEALTH_PAGES 5
#define HEALTH_HR     0
#define HEALTH_SPO2   1
#define HEALTH_STRESS 2
#define HEALTH_STEPS  3
#define HEALTH_BATT   4
static health_ui_t g_health;
static lv_obj_t *g_health_pages[HEALTH_PAGES];
static int g_health_page = 0;
static int g_hr_value, g_spo2_value, g_stress_value, g_steps_value, g_battery_value;
static bool g_power_valid;
static bool g_power_charging;
static bool g_display_sleeping;
static int g_lcd_fd = -1;
static game_ui_t g_game;
static lv_obj_t *g_menu_scr = NULL;
static int g_menu_selected = 0;
static lv_obj_t *g_boot_scr = NULL;
static lv_obj_t *g_menu_title = NULL;
static lv_obj_t *g_menu_touch = NULL;
static lv_timer_t *g_menu_anim = NULL;
static lv_obj_t *g_system_scr = NULL;
static lv_obj_t *g_system_status = NULL;
static bool g_system_armed;
typedef struct{
  lv_obj_t *button;
  lv_obj_t *icon_wrap;
  lv_obj_t *icon;
  lv_obj_t *label;
  honeycomb_cell_t cell;
  int32_t last_icon_size;
  int32_t last_visual_state;
} honey_menu_item_ui_t;
static honey_menu_item_ui_t g_honey_items[8];
static int32_t g_honey_offset_x;
static int32_t g_honey_offset_y;
static int32_t g_honey_velocity_x;
static int32_t g_honey_velocity_y;
static int32_t g_honey_target_x;
static int32_t g_honey_target_y;
static int g_honey_focus;
static int g_honey_target;
static int g_honey_open_target = -1;
static int g_menu_anim_state;
static bool g_honey_dragging;
static bool g_honey_moved;
static uint32_t g_honey_last_tick;
static lv_point_t g_honey_last_point;
#define HONEY_ANIM_IDLE       0
#define HONEY_ANIM_INERTIA    1
#define HONEY_ANIM_SNAP       2
#define HONEY_MENU_ITEMS      8
#define HONEY_SPACING_X       84
#define HONEY_SPACING_Y       72
#define HONEY_ITEM_W          84
#define HONEY_ITEM_H          84
#define HONEY_TOUCH_SLOP      7
#define HONEY_CENTER_Y        8
#define MENU_APP_HEALTH       0
#define MENU_APP_RHYTHM       1
#define MENU_APP_SPORT        2
#define MENU_APP_TIMER        3
#define MENU_APP_SETTINGS     4
#define MENU_APP_WEATHER      5
#define MENU_APP_STEPS        6
#define MENU_APP_ALARM        7
#define APP_ROW_COUNT         3
#define APP_BUTTON_W          128
#define APP_BUTTON_H          52
#define APP_TOUCH_SLOP        18
#define APP_ROWS_TOP_DEFAULT  142
#define APP_ROWS_TOP_HEALTH   236
#define APP_DETAIL_TOP_DEFAULT 176
#define APP_DETAIL_TOP_SETTINGS 96
#define TIMER_MODE_STOPWATCH  0
#define TIMER_MODE_COUNTDOWN  1
#define TIMER_DEFAULT_MS      (5u * 60u * 1000u)
static const lv_image_dsc_t *g_digits[10] = {  &img_digit0, &img_digit1, &img_digit2, &img_digit3, &img_digit4,  &img_digit5, &img_digit6, &img_digit7, &img_digit8, &img_digit9,};
static const char *g_names[NFACES] = { "经典", "米奇", "数字", "运动" };
typedef struct
{
  lv_obj_t *scr;
  lv_obj_t *back;
  lv_obj_t *title;
  lv_obj_t *value;
  lv_obj_t *unit;
  lv_obj_t *detail;
  lv_obj_t *progress_bg;
  lv_obj_t *progress_fill;
  lv_obj_t *rows[APP_ROW_COUNT];
  lv_obj_t *row_labels[APP_ROW_COUNT];
  lv_obj_t *row_values[APP_ROW_COUNT];
  lv_obj_t *action;
  lv_obj_t *action_label;
  lv_obj_t *secondary;
  lv_obj_t *secondary_label;
  lv_obj_t *timer_mode_bar;
  lv_obj_t *timer_stopwatch_mode;
  lv_obj_t *timer_countdown_mode;
  lv_obj_t *footer;
  lv_obj_t *touch;
  lv_obj_t *header;
  lv_obj_t *icon_bg;
  lv_obj_t *icon;
  uint32_t action_color;
  uint32_t secondary_color;
  bool action_style_valid;
  bool secondary_style_valid;
} menu_app_ui_t;
static menu_app_ui_t g_app;
static int g_app_id = -1;
static bool g_app_dragging;
static bool g_app_moved;
static lv_point_t g_app_start_point;
static uint32_t g_app_last_render_ms;
static int g_app_layout_page = -1;
static int g_app_rows_page = -1;
static int32_t g_app_progress_width = -1;
static bool g_timer_running;
static uint32_t g_timer_elapsed_ms;
static uint32_t g_timer_started_ms;
static int g_timer_mode = TIMER_MODE_STOPWATCH;
static uint32_t g_timer_countdown_ms = TIMER_DEFAULT_MS;
static uint32_t g_timer_countdown_total_ms = TIMER_DEFAULT_MS;
static bool g_timer_countdown_running;
static uint32_t g_timer_countdown_started_ms;
static bool g_timer_countdown_expired;
static bool g_sport_running;
static uint32_t g_sport_elapsed_ms;
static uint32_t g_sport_started_ms;
static bool g_rhythm_running;
static uint32_t g_rhythm_started_ms;
static uint32_t g_rhythm_remainder;
static int g_rhythm_bpm = 72;
static uint32_t g_rhythm_beats;
static int g_weather_temp = 23;
static uint32_t g_weather_refreshes;
static bool g_settings[3] = { true, true, true };
static bool g_alarm_enabled = true;
static bool g_alarm_test_active;
static uint32_t g_alarm_test_until_ms;
static bool g_imu_ready;
static uint32_t g_sensor_last_poll_ms;
static void watchface_note_activity(void);
static void watchface_wake_display(void);
static void watchface_sleep_display(void);
static void watchface_update_power(uint32_t now_ms);
/* Forward declarations */
static void build_health_enhanced(void);
static void build_game_enhanced(void);
static void build_menu(void);
static void build_selector(void);
static void build_system(void);
static void build_boot(void);
static void switch_to_face(int idx);
static void open_selector(void);
static void selector_render(void);
static void selector_gesture_cb(lv_event_t *e);
static void selector_select(void);
static void open_system(void);
static void system_restart_cb(lv_event_t *e);
static void gesture_cb(lv_event_t *e);
static void prev_face_cb(lv_event_t *e);
static void next_face_cb(lv_event_t *e);
static void game_tap_cb(lv_event_t *e);
static void boot_done_cb(lv_timer_t *timer);
static void lcd_repaint_cb(lv_timer_t *timer);
static void app_button_cb(int button, int press_type);
static void honey_menu_event_cb(lv_event_t *e);
static void honey_menu_render(void);
static void honey_menu_snap(int index);
static void honey_menu_open(int index);
static void build_rhythm_app(void);
static void build_sport_app(void);
static void build_timer_app(void);
static void build_settings_app(void);
static void build_weather_app(void);
static void build_steps_app(void);
static void build_alarm_app(void);
static void build_menu_app_shell(void);
static void open_menu_app(int index);
static void app_screen_event_cb(lv_event_t *e);
static void app_primary_action(void);
static void app_secondary_action(void);
static void app_render(void);
static void app_tick(uint32_t now_ms);
static void rhythm_update(uint32_t now_ms);
static void return_to_menu(void);
static void return_to_watchface(void);
static void timer_toggle_cb(lv_event_t *e);
static void timer_reset_cb(lv_event_t *e);
static void timer_countdown_update(uint32_t now_ms);
static void timer_set_mode(int mode);
extern int buttons_init(void (*cb)(int button, int press_type));
static uint32_t get_tick_ms(void) {  struct timespec ts;  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint32_t)ts.tv_sec * 1000 + (uint32_t)(ts.tv_nsec / 1000000);}
static void watchface_set_lcd_power(bool on)
{
  if (g_lcd_fd < 0)
    {
      g_lcd_fd = open("/dev/lcd0", O_RDWR | O_NONBLOCK);
    }

  if (g_lcd_fd >= 0)
    {
      (void)ioctl(g_lcd_fd, LCDDEVIO_SETPOWER,
                  on ? CONFIG_LCD_MAXPOWER : 0);
    }
}

static void watchface_wake_display(void)
{
  if (g_disp != NULL)
    {
      lv_display_trigger_activity(g_disp);
    }

  if (g_display_sleeping)
    {
      watchface_set_lcd_power(true);
      if (g_disp != NULL)
        {
          lv_display_enable_invalidation(g_disp, true);
          lv_obj_invalidate(lv_screen_active());
        }
      g_display_sleeping = false;
    }
}

static void watchface_note_activity(void)
{
  watchface_wake_display();
}

static void watchface_sleep_display(void)
{
  if (g_display_sleeping)
    {
      return;
    }

  if (g_disp != NULL)
    {
      lv_display_enable_invalidation(g_disp, false);
    }
  watchface_set_lcd_power(false);
  g_display_sleeping = true;
}

static void watchface_update_power(uint32_t now_ms)
{
  watch_power_snapshot_t snapshot;

  watch_power_poll(now_ms);
  watch_power_get(&snapshot);
  if (snapshot.valid)
    {
      g_power_valid = true;
      g_battery_value = snapshot.battery_percent;
      g_power_charging = snapshot.charging;
    }

  if (watch_power_low_battery_event())
    {
      watch_haptics_pulse(220);
    }
}
static int32_t scale_design_coord(int32_t value) {
  return (int32_t)(((int64_t)value * g_h + FACE_DESIGN_SIZE / 2) /
                   FACE_DESIGN_SIZE);
}
static int32_t scale_454_coord(int32_t value) {
  return (int32_t)(((int64_t)value * g_h + 227) / 454);
}
static void place_design_obj(lv_obj_t *obj, int32_t x, int32_t y) {
  lv_obj_set_pos(obj, (g_w - g_h) / 2 + scale_design_coord(x),
                 scale_design_coord(y));
}
static void get_show_time(struct tm *out) {  struct timespec now;  clock_gettime(CLOCK_MONOTONIC, &now);
  if (g_fake_base < 0) {    g_fake_base = 10 * 3600 + 8 * 60 + 42;    g_boot_mono = now;
  }  time_t elapsed = (now.tv_sec - g_boot_mono.tv_sec);  time_t t = g_fake_base + elapsed;  localtime_r(&t, out);}
static void face_long_press_cb(lv_event_t *e) {
  LV_UNUSED(e);
  watchface_note_activity();
  if (g_mode == MODE_WATCHFACE) {
    open_selector();
  }
}
static void add_common_events(lv_obj_t *scr, int face_idx) {
  lv_obj_t *touch;

  LV_UNUSED(face_idx);
  touch = lv_obj_create(scr);
  lv_obj_remove_style_all(touch);
  lv_obj_set_size(touch, lv_pct(100), lv_pct(100));
  lv_obj_center(touch);
  lv_obj_add_flag(touch, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(touch, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(touch, face_long_press_cb, LV_EVENT_LONG_PRESSED, NULL);
  lv_obj_add_event_cb(touch, gesture_cb, LV_EVENT_GESTURE, NULL);
}
static void switch_to_face(int idx) {
  g_current = ((idx % NFACES) + NFACES) % NFACES;
  g_selecting = false;
  g_mode = MODE_WATCHFACE;
  lv_screen_load(g_face[g_current].scr);
}
static void gesture_cb(lv_event_t *e) {
  LV_UNUSED(e);
  watchface_note_activity();
  lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
  if (dir == LV_DIR_LEFT) {    switch_to_face(g_current + 1);
  } else if (dir == LV_DIR_RIGHT) {    switch_to_face(g_current - 1);
  } else if (dir == LV_DIR_BOTTOM) {    g_mode = MODE_MENU;  
  lv_screen_load(g_menu_scr);    honey_menu_snap(g_menu_selected);
  }}
static void prev_face_cb(lv_event_t *e) {
  LV_UNUSED(e);
  if (g_selecting) {
    g_selector_face = (g_selector_face + NFACES - 1) % NFACES;
    selector_render();
  } else {
    switch_to_face(g_current - 1);
  }}
static void next_face_cb(lv_event_t *e) {
  LV_UNUSED(e);
  if (g_selecting) {
    g_selector_face = (g_selector_face + 1) % NFACES;
    selector_render();
  } else {
    switch_to_face(g_current + 1);
  }}
static int selector_wrap_face(int idx) {
  return ((idx % NFACES) + NFACES) % NFACES;
}
static const lv_image_dsc_t *selector_face_asset(int idx) {
  switch (selector_wrap_face(idx)) {
    case FACE_MICKEY:  return &img_prev_mickey;
    case FACE_DIGITAL: return &img_prev_digital;
    case FACE_SPORT:   return &img_prev_sport;
    case FACE_DIAL:
    default:           return &img_prev_dial;
  }
}
static bool selector_read_point(lv_point_t *point) {
  lv_indev_t *indev = lv_indev_active();
  if (indev == NULL || point == NULL) {
    return false;
  }
  lv_indev_get_point(indev, point);
  return true;
}
static void selector_render(void) {
  if (g_selector_preview_left == NULL || g_selector_preview == NULL ||
      g_selector_preview_right == NULL) {
    return;
  }

  lv_image_set_src(g_selector_preview_left,
                   selector_face_asset(g_selector_face - 1));
  lv_image_set_src(g_selector_preview,
                   selector_face_asset(g_selector_face));
  lv_image_set_src(g_selector_preview_right,
                   selector_face_asset(g_selector_face + 1));
  lv_obj_set_pos(g_selector_preview_left, g_w / 2 - 187, 112);
  lv_obj_set_pos(g_selector_preview, g_w / 2 - 55, 96);
  lv_obj_set_pos(g_selector_preview_right, g_w / 2 + 77, 112);
  lv_obj_set_style_opa(g_selector_preview_left, LV_OPA_60, 0);
  lv_obj_set_style_opa(g_selector_preview, LV_OPA_COVER, 0);
  lv_obj_set_style_opa(g_selector_preview_right, LV_OPA_60, 0);
  if (g_selector_title != NULL) {
    lv_label_set_text(g_selector_title, g_names[g_selector_face]);
  }
}
static void selector_gesture_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_point_t point;

  watchface_note_activity();

  if (code == LV_EVENT_GESTURE) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_LEFT) {
      next_face_cb(NULL);
    } else if (dir == LV_DIR_RIGHT) {
      prev_face_cb(NULL);
    }
    return;
  }

  if (!selector_read_point(&point)) {
    return;
  }

  if (code == LV_EVENT_PRESSED) {
    g_selector_dragging = true;
    g_selector_moved = false;
    g_selector_last_point = point;
  } else if (code == LV_EVENT_PRESSING && g_selector_dragging) {
    if (abs(point.x - g_selector_last_point.x) > 18 ||
        abs(point.y - g_selector_last_point.y) > 18) {
      g_selector_moved = true;
    }
  } else if (code == LV_EVENT_RELEASED && g_selector_dragging) {
    int32_t dx = point.x - g_selector_last_point.x;
    g_selector_dragging = false;
    if (g_selector_moved && abs(dx) > 30) {
      if (dx < 0) {
        next_face_cb(NULL);
      } else {
        prev_face_cb(NULL);
      }
    } else if (!g_selector_moved) {
      selector_select();
    }
  }
}
static void open_selector(void) {
  g_selector_face = g_current;
  g_selecting = true;
  g_mode = MODE_SELECTOR;
  selector_render();
  lv_screen_load(g_selector);
}
static void selector_select(void) {
  switch_to_face(g_selector_face);
}
/* Honeycomb menu ------------------------------------------------------- */
static const char *g_honey_names[HONEY_MENU_ITEMS] = {  "健康", "节拍", "运动", "计时",  "设置", "天气", "步数", "闹钟"};
static const lv_image_dsc_t *g_honey_icons[HONEY_MENU_ITEMS] = {  &img_menu_health, &img_menu_rhythm, &img_ui_activity, &img_menu_timer,  &img_menu_settings, &img_menu_weather, &img_ui_activity, &img_menu_alarm};
static const uint32_t g_honey_colors[HONEY_MENU_ITEMS] = {  0xd9365b, 0x2a9d8f, 0xe76f32, 0x3478c5,  0x64748b, 0x4d86c7, 0x3e9b68, 0xb84a55};
static const honeycomb_cell_t g_honey_cells[HONEY_MENU_ITEMS] = {  { 0, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 },  { -1, 1 }, { 1, -1 }, { -1, 0 }, { -1, -1 }};
static int32_t honey_menu_center_y(void){
  return g_h / 2 + HONEY_CENTER_Y;}
static int honey_menu_nearest(void){
  return honeycomb_nearest_cell(g_honey_cells, HONEY_MENU_ITEMS,                                g_honey_offset_x, g_honey_offset_y,                                HONEY_SPACING_X, HONEY_SPACING_Y);}
static void honey_menu_update_title(void){
  int nearest = honey_menu_nearest();
  if (nearest >= 0)    {      g_honey_focus = nearest;    
  if (g_menu_title != NULL)        {        
  lv_label_set_text(g_menu_title, g_honey_names[nearest]);      
  }  
  }}
static void honey_menu_render(void){
  int center_y = honey_menu_center_y();  honey_menu_update_title();
  for (int i = 0; i < HONEY_MENU_ITEMS; i++)    {      honeycomb_point_t point;      int32_t x;      int32_t y;      int32_t dx;      int32_t dy;      int32_t distance_squared;      int32_t icon_size;      int32_t opacity;      int visual_state;      honeycomb_cell_point(&g_honey_items[i].cell,                           HONEY_SPACING_X, HONEY_SPACING_Y, &point);      x = g_w / 2 + point.x + g_honey_offset_x;      y = center_y + point.y + g_honey_offset_y;      dx = x - g_w / 2;      dy = y - center_y;      distance_squared = dx * dx + dy * dy;
/* Keep a stable hit target while giving the nearest item more weight. */      icon_size = distance_squared < 18 * 18 ? 64 : (distance_squared < 92 * 92 ? 57 : 51);      opacity = 255 - (distance_squared / 64);    
  if (opacity < 75)        {          opacity = 75;      
  }    
  visual_state = i == g_honey_focus && distance_squared < 20 * 20 ? 1 : 0;
  lv_obj_set_pos(g_honey_items[i].button,                     x - HONEY_ITEM_W / 2, y - HONEY_ITEM_H / 2);    
  if (g_honey_items[i].last_icon_size != icon_size) {
    lv_obj_set_size(g_honey_items[i].icon_wrap, icon_size, icon_size);
    g_honey_items[i].last_icon_size = icon_size;
  }
  lv_obj_align(g_honey_items[i].icon_wrap, LV_ALIGN_TOP_MID, 0, 0);    
  lv_obj_set_style_opa(g_honey_items[i].button, (lv_opa_t)opacity, 0);    
  if (g_honey_items[i].last_visual_state != visual_state) {
    g_honey_items[i].last_visual_state = visual_state;
  if (visual_state)        {        
  lv_obj_set_style_border_width(g_honey_items[i].icon_wrap, 2, 0);        
  lv_obj_set_style_border_color(g_honey_items[i].icon_wrap,                                      
  lv_color_hex(0x73e6ff), 0);        
  lv_obj_set_style_bg_color(g_honey_items[i].icon_wrap,                                  
  lv_color_hex(0x132b38), 0);        
  lv_obj_set_style_bg_opa(g_honey_items[i].icon_wrap, LV_OPA_90, 0);        
  lv_obj_set_style_text_color(g_honey_items[i].label,                                    
  lv_color_white(), 0);      
  }      else        {        
  lv_obj_set_style_border_width(g_honey_items[i].icon_wrap, 1, 0);        
  lv_obj_set_style_border_color(g_honey_items[i].icon_wrap,                                      
  lv_color_hex(0x334155), 0);        
  lv_obj_set_style_bg_color(g_honey_items[i].icon_wrap,                                  
  lv_color_hex(g_honey_colors[i]), 0);        
  lv_obj_set_style_bg_opa(g_honey_items[i].icon_wrap, LV_OPA_30, 0);        
  lv_obj_set_style_text_color(g_honey_items[i].label,                                    
  lv_color_hex(0x94a3b8), 0);      
  }  
  }
  }}
static void honey_menu_set_target(int index){
  if (index < 0 || index >= HONEY_MENU_ITEMS)    {      return;  
  }  honeycomb_snap_offset(&g_honey_items[index].cell,                        HONEY_SPACING_X, HONEY_SPACING_Y,                        &g_honey_target_x, &g_honey_target_y);  g_honey_target = index;  g_menu_anim_state = HONEY_ANIM_SNAP;}
static void honey_menu_snap(int index){  g_honey_velocity_x = 0;  g_honey_velocity_y = 0;  honey_menu_set_target(index);}
static int honey_menu_hit(const lv_point_t *point){
  int best = -1;  int32_t best_distance = 0;
  int center_y = honey_menu_center_y();
  for (int i = 0; i < HONEY_MENU_ITEMS; i++)    {      honeycomb_point_t cell_point;      int32_t x;      int32_t y;      int32_t dx;      int32_t dy;      int32_t distance;      honeycomb_cell_point(&g_honey_items[i].cell,                           HONEY_SPACING_X, HONEY_SPACING_Y, &cell_point);      x = g_w / 2 + cell_point.x + g_honey_offset_x;      y = center_y + cell_point.y + g_honey_offset_y;      dx = point->x - x;      dy = point->y - y;      distance = dx * dx + dy * dy;    
  if (best < 0 || distance < best_distance)        {          best = i;          best_distance = distance;      
  }  
  }
  return best_distance <= 52 * 52 ? best : -1;}
static bool honey_menu_read_point(lv_point_t *point){
  lv_indev_t *indev = lv_indev_active();
  if (indev == NULL || point == NULL)    {    
  return false;  
  }
  lv_indev_get_point(indev, point);
  return true;}
static void honey_menu_event_cb(lv_event_t *e){
  lv_event_code_t code = lv_event_get_code(e);
  lv_point_t point;
  watchface_note_activity();
  if (!honey_menu_read_point(&point))    {      return;  
  }
  if (code == LV_EVENT_PRESSED)    {      g_menu_anim_state = HONEY_ANIM_IDLE;      g_honey_dragging = true;      g_honey_moved = false;      g_honey_last_point = point;      g_honey_last_tick = lv_tick_get();      g_honey_velocity_x = 0;      g_honey_velocity_y = 0;  
  }  else if (code == LV_EVENT_PRESSING && g_honey_dragging)    {      int32_t dx = point.x - g_honey_last_point.x;      int32_t dy = point.y - g_honey_last_point.y;      uint32_t now = lv_tick_get();    
  if (abs(point.x - g_honey_last_point.x) >= HONEY_TOUCH_SLOP ||          abs(point.y - g_honey_last_point.y) >= HONEY_TOUCH_SLOP)        {          g_honey_moved = true;      
  }      g_honey_offset_x = honeycomb_clamp(g_honey_offset_x + dx,                                         -HONEY_SPACING_X * 2,                                         HONEY_SPACING_X * 2);      g_honey_offset_y = honeycomb_clamp(g_honey_offset_y + dy,                                         -HONEY_SPACING_Y * 2,                                         HONEY_SPACING_Y * 2);    
  if (now != g_honey_last_tick)        {          g_honey_velocity_x = (g_honey_velocity_x * 2 + dx) / 3;          g_honey_velocity_y = (g_honey_velocity_y * 2 + dy) / 3;      
  }      g_honey_last_point = point;      g_honey_last_tick = now;      honey_menu_render();  
  }  else if (code == LV_EVENT_RELEASED)    {      g_honey_dragging = false;    
  if (!g_honey_moved)        {        
  int hit = honey_menu_hit(&point);        
  if (hit >= 0)            {              honey_menu_snap(hit);              g_menu_selected = hit;              g_honey_open_target = hit;              return;          
  }      
  }    
  if (abs(g_honey_velocity_x) > 1 || abs(g_honey_velocity_y) > 1)        {          g_menu_anim_state = HONEY_ANIM_INERTIA;      
  }      else        {          honey_menu_snap(honey_menu_nearest());      
  }  
  }}
static void honey_menu_anim_cb(lv_timer_t *timer){
  LV_UNUSED(timer);
  if (g_menu_anim_state == HONEY_ANIM_INERTIA)    {      g_honey_offset_x = honeycomb_clamp(g_honey_offset_x + g_honey_velocity_x,                                         -HONEY_SPACING_X * 2,                                         HONEY_SPACING_X * 2);      g_honey_offset_y = honeycomb_clamp(g_honey_offset_y + g_honey_velocity_y,                                         -HONEY_SPACING_Y * 2,                                         HONEY_SPACING_Y * 2);      g_honey_velocity_x = (g_honey_velocity_x * 9) / 10;      g_honey_velocity_y = (g_honey_velocity_y * 9) / 10;    
  if (abs(g_honey_velocity_x) <= 1 && abs(g_honey_velocity_y) <= 1)        {          honey_menu_set_target(honey_menu_nearest());      
  }      honey_menu_render();  
  }  else if (g_menu_anim_state == HONEY_ANIM_SNAP)    {      int32_t dx = g_honey_target_x - g_honey_offset_x;      int32_t dy = g_honey_target_y - g_honey_offset_y;    
  if (abs(dx) <= 1 && abs(dy) <= 1)        {          g_honey_offset_x = g_honey_target_x;          g_honey_offset_y = g_honey_target_y;          g_honey_focus = g_honey_target;          g_menu_anim_state = HONEY_ANIM_IDLE;        
  if (g_honey_open_target >= 0)            {            
  int index = g_honey_open_target;              g_honey_open_target = -1;              honey_menu_open(index);              return;          
  }      
  }      else        {          g_honey_offset_x += dx / 4 != 0 ? dx / 4 : (dx > 0 ? 1 : -1);          g_honey_offset_y += dy / 4 != 0 ? dy / 4 : (dy > 0 ? 1 : -1);      
  }      honey_menu_render();  
  }}
static void honey_menu_open(int index)
{
  if (index < 0 || index >= HONEY_MENU_ITEMS)
    {
      return;
    }

  g_menu_selected = index;
  open_menu_app(index);
}
/* Shared application page ------------------------------------------------
 *
 * The menu opens one light-weight page and swaps its content according to
 * g_app_id. This keeps navigation and touch hit testing identical across all
 * utilities while avoiding eight copies of the same LVGL object tree.
 */
static void app_set_hidden(lv_obj_t *obj, bool hidden)
{
  if (obj == NULL)
    {
      return;
    }

  if (hidden)
    {
      lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
  else
    {
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static void app_set_hidden_if_changed(lv_obj_t *obj, bool hidden)
{
  if (obj == NULL || lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN) == hidden)
    {
      return;
    }

  app_set_hidden(obj, hidden);
}

static void label_set_if_changed(lv_obj_t *label, const char *text)
{
  const char *current;

  if (label == NULL)
    {
      return;
    }

  if (text == NULL)
    {
      text = "";
    }

  current = lv_label_get_text(label);
  if (current == NULL || strcmp(current, text) != 0)
    {
      lv_label_set_text(label, text);
    }
}

static void app_set_progress_width(int32_t width)
{
  if (g_app.progress_fill == NULL || g_app_progress_width == width)
    {
      return;
    }

  g_app_progress_width = width;
  lv_obj_set_size(g_app.progress_fill, width, 12);
}

static bool app_point_in(const lv_point_t *point, lv_obj_t *obj)
{
  lv_area_t area;

  if (point == NULL || obj == NULL || lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN))
    {
      return false;
    }

  lv_obj_get_coords(obj, &area);
  return point->x >= area.x1 && point->x <= area.x2 &&
         point->y >= area.y1 && point->y <= area.y2;
}

static bool app_read_point(lv_point_t *point)
{
  lv_indev_t *indev = lv_indev_active();

  if (indev == NULL || point == NULL)
    {
      return false;
    }

  lv_indev_get_point(indev, point);
  return true;
}

static void app_set_button(lv_obj_t *button, lv_obj_t *label,
                           const char *text, uint32_t color)
{
  bool update_style = true;

  if (button == NULL || label == NULL)
    {
      return;
    }

  label_set_if_changed(label, text);

  if (button == g_app.action)
    {
      update_style = !g_app.action_style_valid ||
                     g_app.action_color != color;
      g_app.action_color = color;
      g_app.action_style_valid = true;
    }
  else if (button == g_app.secondary)
    {
      update_style = !g_app.secondary_style_valid ||
                     g_app.secondary_color != color;
      g_app.secondary_color = color;
      g_app.secondary_style_valid = true;
    }

  if (update_style)
    {
      lv_obj_set_style_bg_color(button, lv_color_hex(color), 0);
      lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    }
}

static void timer_mode_style(lv_obj_t *button, bool selected)
{
  if (button == NULL)
    {
      return;
    }

  lv_obj_set_style_bg_color(button,
                            lv_color_hex(selected ? 0x0e7490 : 0x172235), 0);
  lv_obj_set_style_bg_opa(button, selected ? LV_OPA_COVER : LV_OPA_60, 0);
  lv_obj_set_style_text_color(button,
                              selected ? lv_color_white() :
                                         lv_color_hex(0x94a3b8), 0);
}

static void timer_render_mode_controls(void)
{
  static int last_mode = -1;

  if (g_app.timer_stopwatch_mode == NULL ||
      g_app.timer_countdown_mode == NULL)
    {
      return;
    }

  if (last_mode == g_timer_mode)
    {
      return;
    }

  timer_mode_style(g_app.timer_stopwatch_mode,
                   g_timer_mode == TIMER_MODE_STOPWATCH);
  timer_mode_style(g_app.timer_countdown_mode,
                   g_timer_mode == TIMER_MODE_COUNTDOWN);
  last_mode = g_timer_mode;
}

static void format_duration(uint32_t duration_ms, char *buffer, size_t size)
{
  uint32_t total_seconds = duration_ms / 1000;
  uint32_t minutes = total_seconds / 60;
  uint32_t seconds = total_seconds % 60;
  uint32_t tenths = (duration_ms % 1000) / 100;

  if (minutes > 99)
    {
      minutes = 99;
    }

  snprintf(buffer, size, "%02lu:%02lu.%lu",
           (unsigned long)minutes, (unsigned long)seconds,
           (unsigned long)tenths);
}

static void app_create_row(int index, const char *name)
{
  lv_obj_t *row = g_app.rows[index];
  int32_t rows_top = g_app_id == MENU_APP_HEALTH ? APP_ROWS_TOP_HEALTH : APP_ROWS_TOP_DEFAULT;

  lv_obj_set_size(row, 320, APP_BUTTON_H);
  lv_obj_set_pos(row, (g_w - 320) / 2, rows_top + index * 58);
  lv_obj_set_style_radius(row, 12, 0);
  lv_obj_set_style_bg_color(row, lv_color_hex(0x111b2b), 0);
  lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);

  lv_obj_set_style_text_font(g_app.row_labels[index], &font_zh_16, 0);
  lv_obj_set_style_text_color(g_app.row_labels[index], lv_color_hex(0xcbd5e1), 0);
  label_set_if_changed(g_app.row_labels[index], name);
  lv_obj_align(g_app.row_labels[index], LV_ALIGN_LEFT_MID, 16, 0);

  lv_obj_set_style_text_font(g_app.row_values[index], &font_zh_16, 0);
  lv_obj_set_style_text_color(g_app.row_values[index], lv_color_hex(0x67e8f9), 0);
  lv_obj_align(g_app.row_values[index], LV_ALIGN_RIGHT_MID, -16, 0);
}

static void build_menu_app_shell(void)
{
  if (g_app.scr != NULL)
    {
      return;
    }

  g_app.scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(g_app.scr, lv_color_hex(0x080d16), 0);
  lv_obj_set_style_bg_opa(g_app.scr, LV_OPA_COVER, 0);

  g_app.header = lv_obj_create(g_app.scr);
  lv_obj_remove_style_all(g_app.header);
  lv_obj_set_size(g_app.header, lv_pct(100), 5);
  lv_obj_set_pos(g_app.header, 0, 0);
  lv_obj_set_style_bg_color(g_app.header, lv_color_hex(0x22d3ee), 0);
  lv_obj_set_style_bg_opa(g_app.header, LV_OPA_COVER, 0);

  g_app.back = lv_obj_create(g_app.scr);
  lv_obj_remove_style_all(g_app.back);
  lv_obj_set_size(g_app.back, 72, 42);
  lv_obj_set_pos(g_app.back, 14, 14);
  lv_obj_set_style_radius(g_app.back, 12, 0);
  lv_obj_set_style_bg_color(g_app.back, lv_color_hex(0x172235), 0);
  lv_obj_set_style_bg_opa(g_app.back, LV_OPA_COVER, 0);
  lv_obj_t *back_label = lv_label_create(g_app.back);
  lv_obj_set_style_text_font(back_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(back_label, lv_color_white(), 0);
  lv_label_set_text(back_label, LV_SYMBOL_LEFT);
  lv_obj_center(back_label);

  g_app.title = lv_label_create(g_app.scr);
  lv_obj_set_style_text_font(g_app.title, &font_zh_20, 0);
  lv_obj_set_style_text_color(g_app.title, lv_color_white(), 0);
  lv_obj_align(g_app.title, LV_ALIGN_TOP_MID, 0, 22);

  g_app.icon_bg = lv_obj_create(g_app.scr);
  lv_obj_remove_style_all(g_app.icon_bg);
  lv_obj_set_size(g_app.icon_bg, 44, 44);
  lv_obj_set_pos(g_app.icon_bg, 84, 17);
  lv_obj_set_style_radius(g_app.icon_bg, 14, 0);
  lv_obj_set_style_bg_color(g_app.icon_bg, lv_color_hex(0x164e63), 0);
  lv_obj_set_style_bg_opa(g_app.icon_bg, LV_OPA_COVER, 0);
  g_app.icon = lv_image_create(g_app.icon_bg);
  lv_obj_center(g_app.icon);

  g_app.value = lv_label_create(g_app.scr);
  lv_obj_set_style_text_font(g_app.value, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(g_app.value, lv_color_hex(0x67e8f9), 0);
  lv_obj_align(g_app.value, LV_ALIGN_TOP_MID, 0, 76);

  g_app.unit = lv_label_create(g_app.scr);
  lv_obj_set_style_text_font(g_app.unit, &font_zh_16, 0);
  lv_obj_set_style_text_color(g_app.unit, lv_color_hex(0x94a3b8), 0);
  lv_obj_align(g_app.unit, LV_ALIGN_TOP_MID, 0, 132);

  g_app.detail = lv_label_create(g_app.scr);
  lv_obj_set_style_text_font(g_app.detail, &font_zh_16, 0);
  lv_obj_set_style_text_color(g_app.detail, lv_color_hex(0xcbd5e1), 0);
  lv_obj_set_style_text_align(g_app.detail, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(g_app.detail, 360);
  lv_obj_align(g_app.detail, LV_ALIGN_TOP_MID, 0, APP_DETAIL_TOP_DEFAULT);

  g_app.progress_bg = lv_obj_create(g_app.scr);
  lv_obj_remove_style_all(g_app.progress_bg);
  lv_obj_set_size(g_app.progress_bg, 300, 12);
  lv_obj_align(g_app.progress_bg, LV_ALIGN_TOP_MID, 0, 210);
  lv_obj_set_style_radius(g_app.progress_bg, 6, 0);
  lv_obj_set_style_bg_color(g_app.progress_bg, lv_color_hex(0x1e293b), 0);
  lv_obj_set_style_bg_opa(g_app.progress_bg, LV_OPA_COVER, 0);

  g_app.progress_fill = lv_obj_create(g_app.progress_bg);
  lv_obj_remove_style_all(g_app.progress_fill);
  lv_obj_set_size(g_app.progress_fill, 1, 12);
  lv_obj_set_pos(g_app.progress_fill, 0, 0);
  lv_obj_set_style_radius(g_app.progress_fill, 6, 0);
  lv_obj_set_style_bg_color(g_app.progress_fill, lv_color_hex(0x22d3ee), 0);
  lv_obj_set_style_bg_opa(g_app.progress_fill, LV_OPA_COVER, 0);

  for (int i = 0; i < APP_ROW_COUNT; i++)
    {
      g_app.rows[i] = lv_obj_create(g_app.scr);
      lv_obj_remove_style_all(g_app.rows[i]);
      g_app.row_labels[i] = lv_label_create(g_app.rows[i]);
      lv_obj_remove_style_all(g_app.row_labels[i]);
      g_app.row_values[i] = lv_label_create(g_app.rows[i]);
      lv_obj_remove_style_all(g_app.row_values[i]);
    }

  g_app.action = lv_obj_create(g_app.scr);
  lv_obj_remove_style_all(g_app.action);
  lv_obj_set_size(g_app.action, APP_BUTTON_W, APP_BUTTON_H);
  lv_obj_set_pos(g_app.action, g_w / 2 - APP_BUTTON_W - 8, g_h - 82);
  lv_obj_set_style_radius(g_app.action, 14, 0);
  g_app.action_label = lv_label_create(g_app.action);
  lv_obj_set_style_text_font(g_app.action_label, &font_zh_16, 0);
  lv_obj_set_style_text_color(g_app.action_label, lv_color_white(), 0);
  lv_obj_center(g_app.action_label);

  g_app.secondary = lv_obj_create(g_app.scr);
  lv_obj_remove_style_all(g_app.secondary);
  lv_obj_set_size(g_app.secondary, APP_BUTTON_W, APP_BUTTON_H);
  lv_obj_set_pos(g_app.secondary, g_w / 2 + 8, g_h - 82);
  lv_obj_set_style_radius(g_app.secondary, 14, 0);
  g_app.secondary_label = lv_label_create(g_app.secondary);
  lv_obj_set_style_text_font(g_app.secondary_label, &font_zh_16, 0);
  lv_obj_set_style_text_color(g_app.secondary_label, lv_color_white(), 0);
  lv_obj_center(g_app.secondary_label);

  g_app.timer_mode_bar = lv_obj_create(g_app.scr);
  lv_obj_remove_style_all(g_app.timer_mode_bar);
  lv_obj_set_size(g_app.timer_mode_bar, 244, 40);
  lv_obj_set_pos(g_app.timer_mode_bar, (g_w - 244) / 2, 50);
  lv_obj_set_style_radius(g_app.timer_mode_bar, 18, 0);
  lv_obj_set_style_bg_color(g_app.timer_mode_bar, lv_color_hex(0x0f172a), 0);
  lv_obj_set_style_bg_opa(g_app.timer_mode_bar, LV_OPA_COVER, 0);

  g_app.timer_stopwatch_mode = lv_obj_create(g_app.timer_mode_bar);
  lv_obj_remove_style_all(g_app.timer_stopwatch_mode);
  lv_obj_set_size(g_app.timer_stopwatch_mode, 118, 34);
  lv_obj_set_pos(g_app.timer_stopwatch_mode, 3, 3);
  lv_obj_set_style_radius(g_app.timer_stopwatch_mode, 15, 0);
  lv_obj_add_flag(g_app.timer_stopwatch_mode, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(g_app.timer_stopwatch_mode, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t *stopwatch_label = lv_label_create(g_app.timer_stopwatch_mode);
  lv_obj_set_style_text_font(stopwatch_label, &font_zh_16, 0);
  lv_obj_set_style_text_color(stopwatch_label, lv_color_white(), 0);
  lv_label_set_text(stopwatch_label, "秒表");
  lv_obj_center(stopwatch_label);

  g_app.timer_countdown_mode = lv_obj_create(g_app.timer_mode_bar);
  lv_obj_remove_style_all(g_app.timer_countdown_mode);
  lv_obj_set_size(g_app.timer_countdown_mode, 118, 34);
  lv_obj_set_pos(g_app.timer_countdown_mode, 123, 3);
  lv_obj_set_style_radius(g_app.timer_countdown_mode, 15, 0);
  lv_obj_add_flag(g_app.timer_countdown_mode, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(g_app.timer_countdown_mode, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t *countdown_label = lv_label_create(g_app.timer_countdown_mode);
  lv_obj_set_style_text_font(countdown_label, &font_zh_16, 0);
  lv_obj_set_style_text_color(countdown_label, lv_color_hex(0x94a3b8), 0);
  lv_label_set_text(countdown_label, "倒计时");
  lv_obj_center(countdown_label);
  timer_render_mode_controls();

  g_app.footer = lv_label_create(g_app.scr);
  lv_obj_set_style_text_font(g_app.footer, &font_zh_16, 0);
  lv_obj_set_style_text_color(g_app.footer, lv_color_hex(0x64748b), 0);
  lv_label_set_text(g_app.footer, "按键1  返回    按键2  操作");
  lv_obj_align(g_app.footer, LV_ALIGN_BOTTOM_MID, 0, -12);

  /* Keep input on one stable full-screen object; visual controls stay simple. */
  g_app.touch = lv_obj_create(g_app.scr);
  lv_obj_remove_style_all(g_app.touch);
  lv_obj_set_size(g_app.touch, lv_pct(100), lv_pct(100));
  lv_obj_center(g_app.touch);
  lv_obj_add_flag(g_app.touch, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(g_app.touch, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(g_app.touch, app_screen_event_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(g_app.touch, app_screen_event_cb, LV_EVENT_PRESSING, NULL);
  lv_obj_add_event_cb(g_app.touch, app_screen_event_cb, LV_EVENT_RELEASED, NULL);
  lv_obj_add_event_cb(g_app.touch, app_screen_event_cb, LV_EVENT_GESTURE, NULL);

  app_set_hidden(g_app.progress_bg, true);
  app_set_hidden(g_app.action, true);
  app_set_hidden(g_app.secondary, true);
  app_set_hidden(g_app.timer_mode_bar, true);
  for (int i = 0; i < APP_ROW_COUNT; i++)
    {
      app_set_hidden(g_app.rows[i], true);
    }
}

static void build_rhythm_app(void)
{
  build_menu_app_shell();
}

static void build_sport_app(void)
{
  build_menu_app_shell();
}

static void build_timer_app(void)
{
  build_menu_app_shell();
}

static void build_settings_app(void)
{
  build_menu_app_shell();
}

static void build_weather_app(void)
{
  build_menu_app_shell();
}

static void build_steps_app(void)
{
  build_menu_app_shell();
}

static void build_alarm_app(void)
{
  build_menu_app_shell();
}

static void rhythm_update(uint32_t now_ms)
{
  uint32_t elapsed_ms;
  uint64_t units;

  if (!g_rhythm_running)
    {
      return;
    }

  elapsed_ms = now_ms - g_rhythm_started_ms;
  units = (uint64_t)elapsed_ms * (uint32_t)g_rhythm_bpm +
          (uint64_t)g_rhythm_remainder;
  g_rhythm_beats += (uint32_t)(units / 60000u);
  g_rhythm_remainder = (uint32_t)(units % 60000u);
  g_rhythm_started_ms = now_ms;
}

static uint32_t running_duration(uint32_t stored, bool running,
                                 uint32_t started, uint32_t now)
{
  return stored + (running ? now - started : 0);
}

static uint32_t timer_countdown_remaining(uint32_t now_ms)
{
  uint32_t elapsed_ms;

  if (!g_timer_countdown_running)
    {
      return g_timer_countdown_ms;
    }

  elapsed_ms = now_ms - g_timer_countdown_started_ms;
  if (elapsed_ms >= g_timer_countdown_ms)
    {
      return 0;
    }

  return g_timer_countdown_ms - elapsed_ms;
}

static void timer_countdown_update(uint32_t now_ms)
{
  if (!g_timer_countdown_running)
    {
      return;
    }

  if (now_ms - g_timer_countdown_started_ms >= g_timer_countdown_ms)
    {
      g_timer_countdown_ms = 0;
      g_timer_countdown_running = false;
      g_timer_countdown_started_ms = 0;
      g_timer_countdown_expired = true;
      printf("watchface: countdown complete\n");
    }
}

static void timer_set_mode(int mode)
{
  uint32_t now_ms;

  if (mode != TIMER_MODE_STOPWATCH && mode != TIMER_MODE_COUNTDOWN)
    {
      return;
    }

  if (mode == g_timer_mode)
    {
      timer_render_mode_controls();
      app_render();
      return;
    }

  now_ms = get_tick_ms();
  if (g_timer_mode == TIMER_MODE_STOPWATCH && g_timer_running)
    {
      g_timer_elapsed_ms += now_ms - g_timer_started_ms;
      g_timer_running = false;
      g_timer_started_ms = 0;
    }
  else if (g_timer_mode == TIMER_MODE_COUNTDOWN &&
           g_timer_countdown_running)
    {
      timer_countdown_update(now_ms);
      if (g_timer_countdown_running)
        {
          g_timer_countdown_ms = timer_countdown_remaining(now_ms);
          g_timer_countdown_running = false;
          g_timer_countdown_started_ms = 0;
        }
    }

  g_timer_mode = mode;
  timer_render_mode_controls();
  app_render();
}

static void app_render(void)
{
  char value[32];
  char detail[96];
  char row_value[24];
  uint32_t now = get_tick_ms();
  uint32_t sport_elapsed_ms = 0;
  int progress = 0;
  bool page_changed;
  bool rows_need_layout;

  if (g_app.scr == NULL || g_app_id < 0)
    {
      return;
    }

  page_changed = g_app_layout_page != g_app_id;
  rows_need_layout = g_app_rows_page != g_app_id;
  g_app_last_render_ms = now;

  if (page_changed)
    {
      lv_image_set_src(g_app.icon, g_honey_icons[g_app_id]);
      lv_obj_set_style_bg_color(g_app.header,
                                lv_color_hex(g_honey_colors[g_app_id]), 0);
      lv_obj_set_style_bg_color(g_app.icon_bg,
                                lv_color_hex(g_honey_colors[g_app_id]), 0);
      g_app_progress_width = -1;
    }

  app_set_hidden_if_changed(g_app.value, false);
  app_set_hidden_if_changed(g_app.unit, false);
  app_set_hidden_if_changed(g_app.detail, false);
  app_set_hidden_if_changed(g_app.progress_bg, true);
  app_set_hidden_if_changed(g_app.action, true);
  app_set_hidden_if_changed(g_app.secondary, true);
  for (int i = 0; i < APP_ROW_COUNT; i++)
    {
      app_set_hidden_if_changed(g_app.rows[i], true);
    }

  if (page_changed)
    {
      lv_obj_align(g_app.value, LV_ALIGN_TOP_MID, 0, 76);
      lv_obj_align(g_app.unit, LV_ALIGN_TOP_MID, 0, 132);
      lv_obj_align(g_app.detail, LV_ALIGN_TOP_MID, 0, APP_DETAIL_TOP_DEFAULT);
    }

  app_set_hidden_if_changed(g_app.timer_mode_bar, g_app_id != MENU_APP_TIMER);
  timer_render_mode_controls();

  value[0] = '\0';
  detail[0] = '\0';
  if (page_changed)
    {
      label_set_if_changed(g_app.title, g_honey_names[g_app_id]);
    }

  switch (g_app_id)
    {
      case MENU_APP_HEALTH:
        snprintf(value, sizeof(value), "%d", g_hr_value);
        label_set_if_changed(g_app.unit, "次/分  ·  实时");
        snprintf(detail, sizeof(detail), "血氧 %d%%    压力 %d    电源 %s",
                 g_spo2_value, g_stress_value,
                 g_power_charging ? "CHG" : "电源");
        if (rows_need_layout)
          {
            app_create_row(0, "心率");
            app_create_row(1, "血氧");
            app_create_row(2, "今日步数");
          }
        snprintf(row_value, sizeof(row_value), "%d 次/分", g_hr_value);
        label_set_if_changed(g_app.row_values[0], row_value);
        snprintf(row_value, sizeof(row_value), "%d%%", g_spo2_value);
        label_set_if_changed(g_app.row_values[1], row_value);
        snprintf(row_value, sizeof(row_value), "%d", g_steps_value);
        label_set_if_changed(g_app.row_values[2], row_value);
        for (int i = 0; i < APP_ROW_COUNT; i++)
          {
            app_set_hidden_if_changed(g_app.rows[i], false);
          }
        break;

      case MENU_APP_RHYTHM:
        snprintf(value, sizeof(value), "%d", g_rhythm_bpm);
        label_set_if_changed(g_app.unit, "节拍/分  ·  节拍器");
        snprintf(detail, sizeof(detail), "%s    %lu 拍",
                 g_rhythm_running ? "运行中" : "准备就绪",
                 (unsigned long)g_rhythm_beats);
        app_set_hidden_if_changed(g_app.action, false);
        app_set_hidden_if_changed(g_app.secondary, false);
        app_set_button(g_app.action, g_app.action_label,
                       g_rhythm_running ? "停止" : "开始",
                       g_rhythm_running ? 0xb91c1c : 0x0e7490);
        app_set_button(g_app.secondary, g_app.secondary_label, "加速", 0x155e75);
        break;

      case MENU_APP_SPORT:
        sport_elapsed_ms = running_duration(g_sport_elapsed_ms,
                                            g_sport_running,
                                            g_sport_started_ms, now);
        format_duration(sport_elapsed_ms, value, sizeof(value));
        label_set_if_changed(g_app.unit, "运动训练");
        snprintf(detail, sizeof(detail), "%s    %d 千卡    %d 步",
                 g_sport_running ? "进行中" : "准备就绪",
                 (int)(sport_elapsed_ms / 60000) * 4 + 12,
                 g_steps_value);
        app_set_hidden_if_changed(g_app.action, false);
        app_set_hidden_if_changed(g_app.secondary, false);
        app_set_button(g_app.action, g_app.action_label,
                       g_sport_running ? "暂停" : "开始",
                       g_sport_running ? 0xb45309 : 0x15803d);
        app_set_button(g_app.secondary, g_app.secondary_label, "重置", 0x334155);
        break;

      case MENU_APP_TIMER:
        if (page_changed)
          {
            lv_obj_align(g_app.value, LV_ALIGN_TOP_MID, 0, 92);
            lv_obj_align(g_app.unit, LV_ALIGN_TOP_MID, 0, 150);
          }
        if (g_timer_mode == TIMER_MODE_STOPWATCH)
          {
            format_duration(running_duration(g_timer_elapsed_ms,
                                             g_timer_running,
                                             g_timer_started_ms, now),
                            value, sizeof(value));
            label_set_if_changed(g_app.unit, "秒表");
            snprintf(detail, sizeof(detail), "%s",
                     g_timer_running ? "计时中" : "准备开始");
          }
        else
          {
            uint32_t remaining_ms;

            timer_countdown_update(now);
            remaining_ms = timer_countdown_remaining(now);
            format_duration(remaining_ms, value, sizeof(value));
            label_set_if_changed(g_app.unit, "倒计时  ·  5 分钟");
            if (g_timer_countdown_expired)
              {
                snprintf(detail, sizeof(detail), "时间到");
              }
            else
              {
                snprintf(detail, sizeof(detail), "%s",
                          g_timer_countdown_running ? "倒计时中" :
                                                      "准备开始");
              }
            progress = (int)(((uint64_t)remaining_ms * 100u) /
                             g_timer_countdown_total_ms);
            app_set_hidden_if_changed(g_app.progress_bg, false);
            app_set_progress_width(progress > 0 ? 3 * progress : 1);
          }
        app_set_hidden_if_changed(g_app.action, false);
        app_set_hidden_if_changed(g_app.secondary, false);
        app_set_button(g_app.action, g_app.action_label,
                       g_timer_mode == TIMER_MODE_STOPWATCH ?
                          (g_timer_running ? "暂停" : "开始") :
                          (g_timer_countdown_running ? "暂停" : "开始"),
                       (g_timer_mode == TIMER_MODE_STOPWATCH ? g_timer_running :
                                                               g_timer_countdown_running) ?
                         0xb45309 : 0x0e7490);
        app_set_button(g_app.secondary, g_app.secondary_label, "重置", 0x334155);
        break;

      case MENU_APP_SETTINGS:
        app_set_hidden_if_changed(g_app.value, true);
        app_set_hidden_if_changed(g_app.unit, true);
        if (page_changed)
          {
            lv_obj_align(g_app.detail, LV_ALIGN_TOP_MID, 0, APP_DETAIL_TOP_SETTINGS);
          }
        label_set_if_changed(g_app.detail, "手表设置");
        if (rows_need_layout)
          {
            app_create_row(0, "抬腕亮屏");
            app_create_row(1, "震动");
            app_create_row(2, "自动亮度");
          }
        for (int i = 0; i < APP_ROW_COUNT; i++)
          {
            label_set_if_changed(g_app.row_values[i],
                                 g_settings[i] ? "开" : "关");
            app_set_hidden_if_changed(g_app.rows[i], false);
          }
        break;

      case MENU_APP_WEATHER:
        snprintf(value, sizeof(value), "%d", g_weather_temp);
        label_set_if_changed(g_app.unit, "°C  ·  黄山");
        snprintf(detail, sizeof(detail), "晴朗    最高 28°  最低 17°    已更新 %lu 次",
                 (unsigned long)g_weather_refreshes);
        app_set_hidden_if_changed(g_app.action, false);
        app_set_button(g_app.action, g_app.action_label, "刷新", 0x1d4ed8);
        break;

      case MENU_APP_STEPS:
        snprintf(value, sizeof(value), "%d", g_steps_value);
        label_set_if_changed(g_app.unit, "步数  ·  今日");
        progress = g_steps_value * 100 / 8000;
        if (progress > 100)
          {
            progress = 100;
          }
        snprintf(detail, sizeof(detail), "目标 8000    已完成 %d%%", progress);
        app_set_hidden_if_changed(g_app.progress_bg, false);
        app_set_progress_width(progress > 0 ? 3 * progress : 1);
        app_set_hidden_if_changed(g_app.action, false);
        app_set_button(g_app.action, g_app.action_label, "同步", 0x15803d);
        break;

      case MENU_APP_ALARM:
        snprintf(value, sizeof(value), "%02d:%02d", 7, 30);
        label_set_if_changed(g_app.unit,
                             g_alarm_enabled ? "闹钟已开启" : "闹钟已关闭");
        label_set_if_changed(g_app.detail,
                             g_alarm_test_active ? "闹钟测试：响铃" :
                                                    "工作日    声音  ·  震动");
        app_set_hidden_if_changed(g_app.action, false);
        app_set_hidden_if_changed(g_app.secondary, false);
        app_set_button(g_app.action, g_app.action_label,
                       g_alarm_enabled ? "关闭" : "开启",
                       g_alarm_enabled ? 0xb91c1c : 0x15803d);
        app_set_button(g_app.secondary, g_app.secondary_label, "测试", 0x334155);
        break;

      default:
        break;
    }

  if (value[0] != '\0')
    {
      label_set_if_changed(g_app.value, value);
    }

  if (detail[0] != '\0')
    {
      label_set_if_changed(g_app.detail, detail);
    }

  g_app_layout_page = g_app_id;
  g_app_rows_page = g_app_id;
}

static void app_primary_action(void)
{
  uint32_t now = get_tick_ms();

  switch (g_app_id)
    {
      case MENU_APP_RHYTHM:
        if (g_rhythm_running)
          {
            rhythm_update(now);
            g_rhythm_running = false;
          }
        else
          {
            g_rhythm_started_ms = now;
            g_rhythm_remainder = 0;
            g_rhythm_running = true;
          }
        break;

      case MENU_APP_SPORT:
        if (g_sport_running)
          {
            g_sport_elapsed_ms += now - g_sport_started_ms;
            g_sport_running = false;
          }
        else
          {
            g_sport_started_ms = now;
            g_sport_running = true;
          }
        break;

      case MENU_APP_TIMER:
        if (g_timer_mode == TIMER_MODE_STOPWATCH)
          {
            timer_toggle_cb(NULL);
          }
        else
          {
            uint32_t now_ms = get_tick_ms();

            timer_countdown_update(now_ms);
            if (g_timer_countdown_running)
              {
                g_timer_countdown_ms = timer_countdown_remaining(now_ms);
                g_timer_countdown_running = false;
                g_timer_countdown_started_ms = 0;
              }
            else if (g_timer_countdown_expired ||
                     g_timer_countdown_ms == 0)
              {
                g_timer_countdown_ms = g_timer_countdown_total_ms;
                g_timer_countdown_expired = false;
                g_timer_countdown_started_ms = now_ms;
                g_timer_countdown_running = true;
              }
            else
              {
                g_timer_countdown_started_ms = now_ms;
                g_timer_countdown_running = true;
              }
            app_render();
          }
        return;

      case MENU_APP_WEATHER:
        g_weather_temp = 20 + rand() % 9;
        g_weather_refreshes++;
        break;

      case MENU_APP_STEPS:
        if (g_imu_ready)
          {
            g_steps_value = (int)imu_steps();
          }
        break;

      case MENU_APP_ALARM:
        g_alarm_enabled = !g_alarm_enabled;
        if (!g_alarm_enabled)
          {
            g_alarm_test_active = false;
          }
        break;

      default:
        break;
    }

  app_render();
}

static void app_secondary_action(void)
{
  switch (g_app_id)
    {
      case MENU_APP_RHYTHM:
        if (g_rhythm_running)
          {
            rhythm_update(get_tick_ms());
            g_rhythm_remainder = 0;
          }
        g_rhythm_bpm += 4;
        if (g_rhythm_bpm > 160)
          {
            g_rhythm_bpm = 40;
          }
        break;

      case MENU_APP_SPORT:
        g_sport_running = false;
        g_sport_elapsed_ms = 0;
        break;

      case MENU_APP_TIMER:
        if (g_timer_mode == TIMER_MODE_STOPWATCH)
          {
            timer_reset_cb(NULL);
          }
        else
          {
            g_timer_countdown_running = false;
            g_timer_countdown_started_ms = 0;
            g_timer_countdown_ms = g_timer_countdown_total_ms;
            g_timer_countdown_expired = false;
            app_render();
          }
        return;

      case MENU_APP_ALARM:
        g_alarm_test_active = true;
        g_alarm_test_until_ms = get_tick_ms() + 2000;
        printf("watchface: alarm test\n");
        break;

      default:
        break;
    }

  app_render();
}

static void app_tick(uint32_t now_ms)
{
  if (g_mode != MODE_APP)
    {
      return;
    }

  rhythm_update(now_ms);

  timer_countdown_update(now_ms);

  if (g_alarm_test_active &&
      (int32_t)(now_ms - g_alarm_test_until_ms) >= 0)
    {
      g_alarm_test_active = false;
    }

  /* Keep dynamic pages responsive while cached static content stays cheap. */
  if ((uint32_t)(now_ms - g_app_last_render_ms) >= APP_RENDER_PERIOD_MS)
    {
      app_render();
    }
}

static void timer_toggle_cb(lv_event_t *e)
{
  uint32_t now = get_tick_ms();
  LV_UNUSED(e);

  if (g_timer_mode != TIMER_MODE_STOPWATCH)
    {
      return;
    }

  if (g_timer_running)
    {
      g_timer_elapsed_ms += now - g_timer_started_ms;
      g_timer_running = false;
    }
  else
    {
      g_timer_started_ms = now;
      g_timer_running = true;
    }

  app_render();
}

static void timer_reset_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  g_timer_running = false;
  g_timer_elapsed_ms = 0;
  g_timer_started_ms = 0;
  app_render();
}

static void app_screen_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_point_t point;

  watchface_note_activity();

  if (code == LV_EVENT_GESTURE)
    {
      lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
      if (dir == LV_DIR_RIGHT || dir == LV_DIR_BOTTOM)
        {
          return_to_menu();
        }
      return;
    }

  if (!app_read_point(&point))
    {
      return;
    }

  if (code == LV_EVENT_PRESSED)
    {
      g_app_dragging = true;
      g_app_moved = false;
      g_app_start_point = point;
    }
  else if (code == LV_EVENT_PRESSING && g_app_dragging)
    {
      if (abs(point.x - g_app_start_point.x) > APP_TOUCH_SLOP ||
          abs(point.y - g_app_start_point.y) > APP_TOUCH_SLOP)
        {
          g_app_moved = true;
        }
    }
  else if (code == LV_EVENT_RELEASED && g_app_dragging)
    {
      g_app_dragging = false;
      if (g_app_moved)
        {
          return;
        }

      if (app_point_in(&point, g_app.back))
        {
          return_to_menu();
        }
      else if (g_app_id == MENU_APP_TIMER &&
               app_point_in(&point, g_app.timer_stopwatch_mode))
        {
          timer_set_mode(TIMER_MODE_STOPWATCH);
        }
      else if (g_app_id == MENU_APP_TIMER &&
               app_point_in(&point, g_app.timer_countdown_mode))
        {
          timer_set_mode(TIMER_MODE_COUNTDOWN);
        }
      else if (g_app_id == MENU_APP_SETTINGS)
        {
          for (int i = 0; i < APP_ROW_COUNT; i++)
            {
              if (app_point_in(&point, g_app.rows[i]))
                {
                  g_settings[i] = !g_settings[i];
                  if (i == 1)
                    {
                      watch_haptics_set_enabled(g_settings[i]);
                    }
                  app_render();
                  return;
                }
            }
        }
      else if (app_point_in(&point, g_app.action))
        {
          app_primary_action();
        }
      else if (app_point_in(&point, g_app.secondary))
        {
          app_secondary_action();
        }
    }
}

static void open_menu_app(int index)
{
  if (index < 0 || index >= HONEY_MENU_ITEMS || g_app.scr == NULL)
    {
      return;
    }

  g_app_id = index;
  g_mode = MODE_APP;
  app_render();
  lv_screen_load(g_app.scr);
}

static void return_to_menu(void)
{
  if (g_menu_scr == NULL)
    {
      return_to_watchface();
      return;
    }

  g_mode = MODE_MENU;
  g_app_id = -1;
  lv_screen_load(g_menu_scr);
  honey_menu_snap(g_menu_selected);
}

static void return_to_watchface(void)
{
  g_selecting = false;
  g_mode = MODE_WATCHFACE;
  g_app_id = -1;
  lv_screen_load(g_face[g_current].scr);
}

static void build_menu(void){  g_menu_scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(g_menu_scr, lv_color_hex(0x070b11), 0);
  lv_obj_set_style_bg_opa(g_menu_scr, LV_OPA_COVER, 0);  g_menu_title = lv_label_create(g_menu_scr);
  lv_obj_set_style_text_font(g_menu_title, &font_zh_20, 0);
  lv_obj_set_style_text_color(g_menu_title, lv_color_white(), 0);
  lv_label_set_text(g_menu_title, g_honey_names[0]);
  lv_obj_align(g_menu_title, LV_ALIGN_TOP_MID, 0, 16);
  lv_obj_t *status = lv_label_create(g_menu_scr);
  lv_obj_set_style_text_font(status, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(status, lv_color_hex(0x6ee7b7), 0);
  lv_label_set_text(status, LV_SYMBOL_WIFI);
  lv_obj_align(status, LV_ALIGN_TOP_RIGHT, -18, 20);
  for (int i = 0; i < HONEY_MENU_ITEMS; i++)    {      honey_menu_item_ui_t *item = &g_honey_items[i];      item->cell = g_honey_cells[i];      item->last_icon_size = -1;      item->last_visual_state = -1;      item->button = lv_obj_create(g_menu_scr);    
  lv_obj_remove_style_all(item->button);    
  lv_obj_set_size(item->button, HONEY_ITEM_W, HONEY_ITEM_H);    
  lv_obj_clear_flag(item->button, LV_OBJ_FLAG_SCROLLABLE);      item->icon_wrap = lv_obj_create(item->button);    
  lv_obj_remove_style_all(item->icon_wrap);    
  lv_obj_set_size(item->icon_wrap, 57, 57);    
  lv_obj_set_style_radius(item->icon_wrap, 18, 0);    
  lv_obj_clear_flag(item->icon_wrap, LV_OBJ_FLAG_SCROLLABLE);      item->icon = lv_image_create(item->icon_wrap);    
  lv_image_set_src(item->icon, g_honey_icons[i]);    
  lv_obj_center(item->icon);    
  lv_obj_clear_flag(item->icon, LV_OBJ_FLAG_CLICKABLE);      item->label = lv_label_create(item->button);    
  lv_obj_set_style_text_font(item->label, &font_zh_16, 0);    
  lv_obj_set_style_text_color(item->label, lv_color_hex(0x94a3b8), 0);    
  lv_label_set_text(item->label, g_honey_names[i]);    
  lv_obj_align(item->label, LV_ALIGN_BOTTOM_MID, 0, -1);  
  }  
/* One full-screen input layer makes the gesture path deterministic. */  g_menu_touch = lv_obj_create(g_menu_scr);
  lv_obj_remove_style_all(g_menu_touch);
  lv_obj_set_size(g_menu_touch, lv_pct(100), lv_pct(100));
  lv_obj_center(g_menu_touch);
  lv_obj_add_flag(g_menu_touch, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(g_menu_touch, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(g_menu_touch, honey_menu_event_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(g_menu_touch, honey_menu_event_cb, LV_EVENT_PRESSING, NULL);
  lv_obj_add_event_cb(g_menu_touch, honey_menu_event_cb, LV_EVENT_RELEASED, NULL);
  lv_obj_t *hint = lv_label_create(g_menu_scr);
  lv_obj_set_style_text_font(hint, &font_zh_16, 0);
  lv_obj_set_style_text_color(hint, lv_color_hex(0x64748b), 0);
  lv_label_set_text(hint, "按键1 返回   按键2 打开");
  lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -12);  g_honey_offset_x = 0;  g_honey_offset_y = 0;  g_honey_focus = 0;  g_honey_target = 0;  g_honey_open_target = -1;  g_menu_anim_state = HONEY_ANIM_IDLE;  g_menu_anim = lv_timer_create(honey_menu_anim_cb, HONEY_MENU_ANIM_PERIOD_MS, NULL);  honey_menu_render();}
/* Game functions */
static void game_spawn_note(void) {
  int i;
  for (i = 0; i < GAME_NOTES_MAX; i++) {  
  if (!g_game.notes[i].active) break;
  }
  if (i >= GAME_NOTES_MAX) return;
  int angle = rand() % 360;
  int radius = 200;  g_game.notes[i].cx = g_w / 2 + (int)(radius * cos(angle * M_PI / 180));  g_game.notes[i].cy = g_h / 2 + (int)(radius * sin(angle * M_PI / 180));  g_game.notes[i].spawn_tick = get_tick_ms();  g_game.notes[i].active = true;
  lv_obj_set_pos(g_game.notes[i].obj, g_game.notes[i].cx - GAME_NOTE_R, g_game.notes[i].cy - GAME_NOTE_R);
  lv_obj_clear_flag(g_game.notes[i].obj, LV_OBJ_FLAG_HIDDEN);}
static void game_update_notes(void) {  uint32_t now = get_tick_ms();
  for (int i = 0; i < GAME_NOTES_MAX; i++) {  
  if (!g_game.notes[i].active) continue;    float progress = (float)(now - g_game.notes[i].spawn_tick) / GAME_SHRINK_MS;  
  if (progress >= 1.0f) {      g_game.notes[i].active = false;    
  lv_obj_add_flag(g_game.notes[i].obj, LV_OBJ_FLAG_HIDDEN);      g_game.combo = 0;    
  lv_label_set_text(g_game.lbl_combo, "");    
  lv_label_set_text(g_game.lbl_judge, "MISS");    
  lv_obj_set_style_text_color(g_game.lbl_judge, lv_color_hex(0xff0000), 0);      continue;  
  }    int32_t x = g_game.notes[i].cx + (int32_t)((g_w / 2 - g_game.notes[i].cx) * progress);    int32_t y = g_game.notes[i].cy + (int32_t)((g_h / 2 - g_game.notes[i].cy) * progress);  
  lv_obj_set_pos(g_game.notes[i].obj, x - GAME_NOTE_R, y - GAME_NOTE_R);
  }}
static void game_tap_cb(lv_event_t *e) {
  LV_UNUSED(e);
  watchface_note_activity();
  lv_indev_t *indev = lv_indev_active();
  lv_point_t point;
  lv_indev_get_point(indev, &point);  int32_t tx = point.x, ty = point.y;
  for (int i = 0; i < GAME_NOTES_MAX; i++) {  
  if (!g_game.notes[i].active) continue;    float progress = (float)(get_tick_ms() - g_game.notes[i].spawn_tick) / GAME_SHRINK_MS;    int32_t x = g_game.notes[i].cx + (int32_t)((g_w / 2 - g_game.notes[i].cx) * progress);    int32_t y = g_game.notes[i].cy + (int32_t)((g_h / 2 - g_game.notes[i].cy) * progress);    int32_t dx = tx - x, dy = ty - y;    int32_t dist = (int32_t)sqrt(dx * dx + dy * dy);  
  if (dist < GAME_HIT_ZONE) {      g_game.notes[i].active = false;    
  lv_obj_add_flag(g_game.notes[i].obj, LV_OBJ_FLAG_HIDDEN);      int32_t score = (int32_t)(100 * (1.0f - progress));      g_game.score += score;      g_game.combo++;    
  if (g_game.combo > g_game.max_combo) g_game.max_combo = g_game.combo;    
  char buf[32];      snprintf(buf, sizeof(buf), "%lu", (unsigned long)g_game.score);    
  lv_label_set_text(g_game.lbl_score, buf);    
  if (g_game.combo > 1) {        snprintf(buf, sizeof(buf), "%lu combo", (unsigned long)g_game.combo);      
  lv_label_set_text(g_game.lbl_combo, buf);    
  }    
  if (progress < 0.3f) {      
  lv_label_set_text(g_game.lbl_judge, "PERFECT!");      
  lv_obj_set_style_text_color(g_game.lbl_judge, lv_color_hex(0x00ff88), 0);    
  } else if (progress < 0.6f) {      
  lv_label_set_text(g_game.lbl_judge, "GREAT!");      
  lv_obj_set_style_text_color(g_game.lbl_judge, lv_color_hex(0x00aaff), 0);    
  } else {      
  lv_label_set_text(g_game.lbl_judge, "GOOD");      
  lv_obj_set_style_text_color(g_game.lbl_judge, lv_color_hex(0xffaa00), 0);    
  }      break;  
  }
  }}
/* Particle system */
#define MAX_PARTICLES 20
typedef struct {
  lv_obj_t *obj;  int32_t vx, vy;  uint32_t life;  bool active;} particle_t;
static particle_t g_particles[MAX_PARTICLES];
static void init_particles(void) {
  for (int i = 0; i < MAX_PARTICLES; i++) {    g_particles[i].obj = lv_obj_create(g_game.scr);  
  lv_obj_remove_style_all(g_particles[i].obj);  
  lv_obj_set_size(g_particles[i].obj, 6, 6);  
  lv_obj_set_style_radius(g_particles[i].obj, 3, 0);  
  lv_obj_set_style_bg_color(g_particles[i].obj, lv_color_hex(0x06b6d4), 0);  
  lv_obj_set_style_bg_opa(g_particles[i].obj, LV_OPA_COVER, 0);  
  lv_obj_add_flag(g_particles[i].obj, LV_OBJ_FLAG_HIDDEN);    g_particles[i].active = false;
  }}
static void spawn_hit_particles(int32_t x, int32_t y, lv_color_t color) {
  for (int i = 0; i < MAX_PARTICLES; i++) {  
  if (!g_particles[i].active) {      g_particles[i].active = true;      g_particles[i].vx = (rand() % 200 - 100) * 2;      g_particles[i].vy = (rand() % 200 - 100) * 2;      g_particles[i].life = 500;    
  lv_obj_set_pos(g_particles[i].obj, x - 3, y - 3);    
  lv_obj_set_style_bg_color(g_particles[i].obj, color, 0);    
  lv_obj_clear_flag(g_particles[i].obj, LV_OBJ_FLAG_HIDDEN);  
  }
  }}
static void update_health_display(void) {
  char buf[64];  snprintf(buf, sizeof(buf), "%d", g_hr_value);
  lv_label_set_text(g_health.lbl_hr, buf);  snprintf(buf, sizeof(buf), "%d%%", g_spo2_value);
  lv_label_set_text(g_health.lbl_spo2, buf);  snprintf(buf, sizeof(buf), "%d", g_stress_value);
  lv_label_set_text(g_health.lbl_stress, buf);  snprintf(buf, sizeof(buf), "%d", g_steps_value);
  lv_label_set_text(g_health.lbl_steps, buf);  snprintf(buf, sizeof(buf), "%d%%", g_battery_value);
  lv_label_set_text(g_health.lbl_battery, buf);
  if (g_health.lbl_power_status != NULL)
    {
      lv_label_set_text(g_health.lbl_power_status,
                        g_power_charging ? LV_SYMBOL_CHARGE : "电源");
    }
}
/* Face building functions */
static void build_dial_face(face_ui_t *ui) {
  ui->scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ui->scr, lv_color_black(), 0);

  ui->bg = lv_img_create(ui->scr);
  lv_img_set_src(ui->bg, &img_official_dial_bg);
  lv_obj_center(ui->bg);

  ui->hour = lv_img_create(ui->scr);
  lv_img_set_src(ui->hour, &img_official_dial_hour);
  lv_img_set_pivot(ui->hour, scale_design_coord(17), scale_design_coord(108));
  lv_obj_set_pos(ui->hour, g_w / 2 - scale_design_coord(17),
                 g_h / 2 - scale_design_coord(108));

  ui->min = lv_img_create(ui->scr);
  lv_img_set_src(ui->min, &img_official_dial_minute);
  lv_img_set_pivot(ui->min, scale_design_coord(20), scale_design_coord(140));
  lv_obj_set_pos(ui->min, g_w / 2 - scale_design_coord(20),
                 g_h / 2 - scale_design_coord(140));

  ui->sec = lv_img_create(ui->scr);
  lv_img_set_src(ui->sec, &img_official_dial_second);
  lv_img_set_pivot(ui->sec, scale_design_coord(8), scale_design_coord(160));
  lv_obj_set_pos(ui->sec, g_w / 2 - scale_design_coord(8),
                 g_h / 2 - scale_design_coord(160));
  add_common_events(ui->scr, FACE_DIAL);
}
static void build_mickey_face(face_ui_t *ui) {
  ui->scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ui->scr, lv_color_black(), 0);

  ui->bg = lv_img_create(ui->scr);
  lv_img_set_src(ui->bg, &img_mickey_bg);
  lv_obj_center(ui->bg);

  ui->mickey_body = lv_img_create(ui->scr);
  lv_img_set_src(ui->mickey_body, &img_body);
  place_design_obj(ui->mickey_body, 112, 200);

  ui->mickey_head = lv_img_create(ui->scr);
  lv_img_set_src(ui->mickey_head, &img_head);
  place_design_obj(ui->mickey_head, 134, 60);

  ui->mickey_eye = lv_img_create(ui->scr);
  lv_img_set_src(ui->mickey_eye, &img_eye0);
  place_design_obj(ui->mickey_eye, 194, 136);
  lv_obj_add_flag(ui->mickey_eye, LV_OBJ_FLAG_HIDDEN);

  ui->mickey_hour = lv_img_create(ui->scr);
  lv_img_set_src(ui->mickey_hour, &img_mickey_hand_hour);
  lv_img_set_pivot(ui->mickey_hour, scale_454_coord(161),
                   scale_454_coord(36));
  lv_obj_set_pos(ui->mickey_hour, g_w / 2 - scale_454_coord(161),
                 g_h / 2 - scale_454_coord(36));

  ui->mickey_minute = lv_img_create(ui->scr);
  lv_img_set_src(ui->mickey_minute, &img_mickey_hand_minute);
  lv_img_set_pivot(ui->mickey_minute, scale_454_coord(29),
                   scale_454_coord(178));
  lv_obj_set_pos(ui->mickey_minute, g_w / 2 - scale_454_coord(29),
                 g_h / 2 - scale_454_coord(178));

  ui->lbl_time = lv_label_create(ui->scr);
  lv_obj_set_style_text_font(ui->lbl_time, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(ui->lbl_time, lv_color_white(), 0);
  lv_label_set_text(ui->lbl_time, "10:08");
  lv_obj_align(ui->lbl_time, LV_ALIGN_BOTTOM_MID, 0, -20);
  add_common_events(ui->scr, FACE_MICKEY);
}
static void build_digital_face(face_ui_t *ui) {
  int32_t digit_width = (int32_t)img_digit0.header.w;
  int32_t digit_gap = 4;
  int32_t colon_width = 16;
  int32_t total_width = digit_width * 4 + digit_gap * 4 + colon_width;
  int32_t start_x = (g_w - total_width) / 2;
  int32_t start_y = scale_design_coord(174);
  ui->scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ui->scr, lv_color_black(), 0);

  ui->bg = lv_img_create(ui->scr);
  lv_img_set_src(ui->bg, &img_digital_bg);
  lv_obj_center(ui->bg);

  for (int i = 0; i < 4; i++) {
    ui->digits[i] = lv_img_create(ui->scr);
    lv_img_set_src(ui->digits[i], g_digits[0]);
    int32_t x = start_x + i * (digit_width + digit_gap);
    if (i >= 2) {
      x += colon_width;
    }
    lv_obj_set_pos(ui->digits[i], x, start_y);
  }

  ui->lbl_colon = lv_label_create(ui->scr);
  lv_obj_set_style_text_font(ui->lbl_colon, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(ui->lbl_colon, lv_color_hex(0x06b6d4), 0);
  lv_label_set_text(ui->lbl_colon, ":");
  lv_obj_set_width(ui->lbl_colon, colon_width);
  lv_obj_set_pos(ui->lbl_colon, start_x + digit_width * 2 + digit_gap * 2,
                 start_y + 3);

  ui->lbl_date = lv_label_create(ui->scr);
  lv_obj_set_style_text_font(ui->lbl_date, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(ui->lbl_date, lv_color_hex(0x94a3b8), 0);
  lv_label_set_text(ui->lbl_date, "2026/08/30");
  lv_obj_align(ui->lbl_date, LV_ALIGN_CENTER, 0, 52);

  ui->lbl_week = lv_label_create(ui->scr);
  lv_obj_set_style_text_font(ui->lbl_week, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(ui->lbl_week, lv_color_hex(0x64748b), 0);
  lv_label_set_text(ui->lbl_week, "Sat");
  lv_obj_align(ui->lbl_week, LV_ALIGN_CENTER, 0, 80);
  add_common_events(ui->scr, FACE_DIGITAL);
}
static void build_sport_face(face_ui_t *ui) {
  int32_t sport_pivot_x = (int32_t)img_official_sport_hour.header.w / 2;
  int32_t sport_pivot_y = (int32_t)img_official_sport_hour.header.h / 2;
  ui->scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ui->scr, lv_color_black(), 0);

  ui->bg = lv_img_create(ui->scr);
  lv_img_set_src(ui->bg, &img_sport_bg);
  lv_obj_center(ui->bg);

  ui->sport_hour = lv_img_create(ui->scr);
  lv_img_set_src(ui->sport_hour, &img_official_sport_hour);
  lv_img_set_pivot(ui->sport_hour, sport_pivot_x, sport_pivot_y);
  lv_obj_set_pos(ui->sport_hour, g_w / 2 - sport_pivot_x,
                 g_h / 2 - sport_pivot_y);

  ui->sport_minute = lv_img_create(ui->scr);
  lv_img_set_src(ui->sport_minute, &img_official_sport_minute);
  lv_img_set_pivot(ui->sport_minute, sport_pivot_x, sport_pivot_y);
  lv_obj_set_pos(ui->sport_minute, g_w / 2 - sport_pivot_x,
                 g_h / 2 - sport_pivot_y);

  ui->sport_second = lv_img_create(ui->scr);
  lv_img_set_src(ui->sport_second, &img_official_sport_second);
  lv_img_set_pivot(ui->sport_second, sport_pivot_x, sport_pivot_y);
  lv_obj_set_pos(ui->sport_second, g_w / 2 - sport_pivot_x,
                 g_h / 2 - sport_pivot_y);

  ui->lbl_steps = lv_label_create(ui->scr);
  lv_obj_set_style_text_font(ui->lbl_steps, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(ui->lbl_steps, lv_color_white(), 0);
  lv_label_set_text(ui->lbl_steps, "0 steps");
  lv_obj_align(ui->lbl_steps, LV_ALIGN_BOTTOM_MID, 0, -48);
  add_common_events(ui->scr, FACE_SPORT);
}
/* Health screen */
static void build_health_enhanced(void) {
  for (int i = 0; i < HEALTH_PAGES; i++) {    g_health_pages[i] = lv_obj_create(NULL);  
  lv_obj_set_style_bg_color(g_health_pages[i], lv_color_hex(0x0b1120), 0);
  }  g_health.scr = g_health_pages[0];    g_health.lbl_hr = lv_label_create(g_health_pages[HEALTH_HR]);
  lv_obj_set_style_text_font(g_health.lbl_hr, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(g_health.lbl_hr, lv_color_hex(0xef4444), 0);
  lv_label_set_text(g_health.lbl_hr, "72");
  lv_obj_center(g_health.lbl_hr);    g_health.lbl_spo2 = lv_label_create(g_health_pages[HEALTH_SPO2]);
  lv_obj_set_style_text_font(g_health.lbl_spo2, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(g_health.lbl_spo2, lv_color_hex(0x60a5fa), 0);
  lv_label_set_text(g_health.lbl_spo2, "98%");
  lv_obj_center(g_health.lbl_spo2);    g_health.lbl_stress = lv_label_create(g_health_pages[HEALTH_STRESS]);
  lv_obj_set_style_text_font(g_health.lbl_stress, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(g_health.lbl_stress, lv_color_hex(0xfbbf24), 0);
  lv_label_set_text(g_health.lbl_stress, "45");
  lv_obj_center(g_health.lbl_stress);    g_health.lbl_steps = lv_label_create(g_health_pages[HEALTH_STEPS]);
  lv_obj_set_style_text_font(g_health.lbl_steps, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(g_health.lbl_steps, lv_color_hex(0x4ade80), 0);
  lv_label_set_text(g_health.lbl_steps, "0");
  lv_obj_center(g_health.lbl_steps);    g_health.lbl_battery = lv_label_create(g_health_pages[HEALTH_BATT]);
  lv_obj_set_style_text_font(g_health.lbl_battery, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(g_health.lbl_battery, lv_color_hex(0x94a3b8), 0);
  lv_label_set_text(g_health.lbl_battery, "--%");
  lv_obj_center(g_health.lbl_battery);
  g_health.lbl_power_status = lv_label_create(g_health_pages[HEALTH_BATT]);
  lv_obj_set_style_text_font(g_health.lbl_power_status, &font_zh_16, 0);
  lv_obj_set_style_text_color(g_health.lbl_power_status,
                              lv_color_hex(0x6ee7b7), 0);
  lv_label_set_text(g_health.lbl_power_status, "电源");
  lv_obj_align(g_health.lbl_power_status, LV_ALIGN_CENTER, 0, 48);
  g_health.lbl_time = lv_label_create(g_health_pages[0]);
  lv_obj_set_style_text_font(g_health.lbl_time, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(g_health.lbl_time, lv_color_hex(0x94a3b8), 0);
  lv_obj_align(g_health.lbl_time, LV_ALIGN_TOP_RIGHT, -10, 5);    g_health.lbl_date = lv_label_create(g_health_pages[0]);
  lv_obj_set_style_text_font(g_health.lbl_date, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(g_health.lbl_date, lv_color_hex(0x64748b), 0);
  lv_obj_align(g_health.lbl_date, LV_ALIGN_TOP_RIGHT, -10, 28);}
/* Game screen */
static void build_game_enhanced(void) {  g_game.scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(g_game.scr, lv_color_black(), 0);    g_game.target_ring = lv_obj_create(g_game.scr);
  lv_obj_remove_style_all(g_game.target_ring);
  lv_obj_set_size(g_game.target_ring, GAME_TARGET_R * 2, GAME_TARGET_R * 2);
  lv_obj_set_style_radius(g_game.target_ring, GAME_TARGET_R, 0);
  lv_obj_set_style_border_width(g_game.target_ring, 3, 0);
  lv_obj_set_style_border_color(g_game.target_ring, lv_color_hex(0x06b6d4), 0);
  lv_obj_center(g_game.target_ring);  
  lv_obj_t *tap = lv_obj_create(g_game.scr);
  lv_obj_remove_style_all(tap);
  lv_obj_set_size(tap, lv_pct(100), lv_pct(100));
  lv_obj_center(tap);
  lv_obj_add_flag(tap, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(tap, game_tap_cb, LV_EVENT_CLICKED, NULL);  
  for (int i = 0; i < GAME_NOTES_MAX; i++) {    g_game.notes[i].obj = lv_obj_create(g_game.scr);  
  lv_obj_remove_style_all(g_game.notes[i].obj);  
  lv_obj_set_size(g_game.notes[i].obj, GAME_NOTE_R * 2, GAME_NOTE_R * 2);  
  lv_obj_set_style_radius(g_game.notes[i].obj, GAME_NOTE_R, 0);  
  lv_obj_set_style_bg_color(g_game.notes[i].obj, lv_color_hex(0x06b6d4), 0);  
  lv_obj_add_flag(g_game.notes[i].obj, LV_OBJ_FLAG_HIDDEN);    g_game.notes[i].active = false;
  }    g_game.lbl_score = lv_label_create(g_game.scr);
  lv_obj_set_style_text_font(g_game.lbl_score, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(g_game.lbl_score, lv_color_white(), 0);
  lv_label_set_text(g_game.lbl_score, "0");
  lv_obj_align(g_game.lbl_score, LV_ALIGN_TOP_MID, 0, 16);    g_game.lbl_combo = lv_label_create(g_game.scr);
  lv_obj_set_style_text_font(g_game.lbl_combo, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(g_game.lbl_combo, lv_color_hex(0x06b6d4), 0);
  lv_label_set_text(g_game.lbl_combo, "");
  lv_obj_align(g_game.lbl_combo, LV_ALIGN_TOP_MID, 0, 56);    g_game.lbl_judge = lv_label_create(g_game.scr);
  lv_obj_set_style_text_font(g_game.lbl_judge, &lv_font_montserrat_36, 0);
  lv_obj_set_style_text_color(g_game.lbl_judge, lv_color_hex(0x00ff88), 0);
  lv_label_set_text(g_game.lbl_judge, "");
  lv_obj_align(g_game.lbl_judge, LV_ALIGN_CENTER, 0, 80);    init_particles();}
/* Face selector: a three-card carousel, matching the watchOS face editor. */
static void build_selector(void) {
  g_selector = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(g_selector, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(g_selector, LV_OPA_COVER, 0);

  g_selector_title = lv_label_create(g_selector);
  lv_obj_set_style_text_font(g_selector_title, &font_zh_20, 0);
  lv_obj_set_style_text_color(g_selector_title, lv_color_white(), 0);
  lv_obj_align(g_selector_title, LV_ALIGN_TOP_MID, 0, 24);

  g_selector_preview_left = lv_image_create(g_selector);
  g_selector_preview = lv_image_create(g_selector);
  g_selector_preview_right = lv_image_create(g_selector);
  lv_obj_set_style_radius(g_selector_preview, 12, 0);
  lv_obj_set_style_border_width(g_selector_preview, 2, 0);
  lv_obj_set_style_border_color(g_selector_preview, lv_color_hex(0x38bdf8), 0);
  lv_obj_set_style_radius(g_selector_preview_left, 10, 0);
  lv_obj_set_style_radius(g_selector_preview_right, 10, 0);

  g_selector_touch = lv_obj_create(g_selector);
  lv_obj_remove_style_all(g_selector_touch);
  lv_obj_set_size(g_selector_touch, lv_pct(100), lv_pct(100));
  lv_obj_center(g_selector_touch);
  lv_obj_add_flag(g_selector_touch, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(g_selector_touch, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(g_selector_touch, selector_gesture_cb,
                      LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(g_selector_touch, selector_gesture_cb,
                      LV_EVENT_PRESSING, NULL);
  lv_obj_add_event_cb(g_selector_touch, selector_gesture_cb,
                      LV_EVENT_RELEASED, NULL);
  lv_obj_add_event_cb(g_selector_touch, selector_gesture_cb,
                      LV_EVENT_GESTURE, NULL);

  g_selector_face = g_current;
  selector_render();
}

static void build_system(void) {
  lv_obj_t *title;
  lv_obj_t *restart;
  lv_obj_t *restart_label;

  g_system_scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(g_system_scr, lv_color_hex(0x090d14), 0);
  lv_obj_set_style_bg_opa(g_system_scr, LV_OPA_COVER, 0);

  title = lv_label_create(g_system_scr);
  lv_obj_set_style_text_font(title, &font_zh_20, 0);
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_label_set_text(title, "电源");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 44);

  restart = lv_obj_create(g_system_scr);
  lv_obj_remove_style_all(restart);
  lv_obj_set_size(restart, 180, 76);
  lv_obj_set_style_radius(restart, 16, 0);
  lv_obj_set_style_bg_color(restart, lv_color_hex(0x7f1d1d), 0);
  lv_obj_set_style_bg_opa(restart, LV_OPA_COVER, 0);
  lv_obj_center(restart);
  lv_obj_add_flag(restart, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(restart, system_restart_cb, LV_EVENT_CLICKED, NULL);

  restart_label = lv_label_create(restart);
  lv_obj_set_style_text_font(restart_label, &font_zh_20, 0);
  lv_obj_set_style_text_color(restart_label, lv_color_white(), 0);
  lv_label_set_text(restart_label, "重启");
  lv_obj_center(restart_label);

  g_system_status = lv_label_create(g_system_scr);
  lv_obj_set_style_text_font(g_system_status, &font_zh_16, 0);
  lv_obj_set_style_text_color(g_system_status, lv_color_hex(0x94a3b8), 0);
  lv_label_set_text(g_system_status, "再次点击确认");
  lv_obj_align(g_system_status, LV_ALIGN_CENTER, 0, 92);
}
static void open_system(void) {
  g_system_armed = false;
  g_mode = MODE_SYSTEM;
  if (g_system_status != NULL) {
    lv_label_set_text(g_system_status, "再次点击确认");
  }
  lv_screen_load(g_system_scr);
}
static void system_restart_cb(lv_event_t *e) {
  LV_UNUSED(e);
  watchface_note_activity();
  if (g_mode != MODE_SYSTEM) {
    return;
  }

  if (!g_system_armed) {
    g_system_armed = true;
    lv_label_set_text(g_system_status, "再次点击重启");
  } else {
    extern void up_systemreset(void);
    printf("watchface: confirmed system restart\n");
    up_systemreset();
  }
}
/* Boot */
static void build_boot(void) {  g_boot_scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(g_boot_scr, lv_color_hex(0x0b1120), 0);
  lv_obj_t *logo = lv_label_create(g_boot_scr);
  lv_obj_set_style_text_font(logo, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(logo, lv_color_hex(0x06b6d4), 0);
  lv_label_set_text(logo, "*");
  lv_obj_center(logo);
  lv_obj_t *name = lv_label_create(g_boot_scr);
  lv_obj_set_style_text_font(name, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(name, lv_color_hex(0xf8fafc), 0);
  lv_label_set_text(name, "BeatPulse");
  lv_obj_align(name, LV_ALIGN_CENTER, 0, 30);}
static void boot_done_cb(lv_timer_t *timer) {
  LV_UNUSED(timer);
  lv_screen_load(g_face[g_current].scr);  g_mode = MODE_WATCHFACE;}
/* The board registers /dev/lcd0 before its asynchronous panel setup is * complete. Re-invalidate the active screen while that setup settles so a * first frame dropped during the race cannot leave the panel blank. */
static void lcd_repaint_cb(lv_timer_t *timer) {
  lv_obj_t *active = lv_screen_active();
  LV_UNUSED(timer);
  if (active != NULL) {  
  lv_obj_invalidate(active);
  }}
/* Button callback */
static void app_button_cb(int button, int press_type) {
  if (button != 0 && button != 1) {
    return;
  }

  watchface_note_activity();

  if (press_type == PRESS_LONG) {
    if (button == 0) {
      open_system();
    }
    return;
  }

  if (press_type != PRESS_SHORT) {
    return;
  }

  if (button == 0) {
    if (g_mode == MODE_APP) {
      return_to_menu();
    } else if (g_mode == MODE_MENU) {
      return_to_watchface();
    } else if (g_mode != MODE_WATCHFACE) {
      return_to_watchface();
    }
  } else if (g_mode == MODE_SYSTEM) {
    system_restart_cb(NULL);
  } else if (g_mode == MODE_SELECTOR) {
    selector_select();
  } else if (g_mode == MODE_MENU) {
    g_honey_open_target = g_honey_focus;
    honey_menu_snap(g_honey_focus);
  } else if (g_mode == MODE_APP) {
    app_primary_action();
  } else if (g_mode == MODE_WATCHFACE && g_menu_scr != NULL) {
    g_mode = MODE_MENU;
    lv_screen_load(g_menu_scr);
    honey_menu_snap(g_menu_selected);
  }
}
/* Face tick */
static void face_tick(lv_timer_t *timer) {
  struct tm t;
  uint32_t tick_start_ms = get_tick_ms();
  LV_UNUSED(timer);
  get_show_time(&t);
  watchface_update_power(tick_start_ms);
  watch_haptics_poll(tick_start_ms);

  if (g_imu_ready &&
      (g_sensor_last_poll_ms == 0 ||
       tick_start_ms - g_sensor_last_poll_ms >= SENSOR_POLL_PERIOD_MS))
    {
      imu_poll();
      g_sensor_last_poll_ms = tick_start_ms;
      g_steps_value = (int)imu_steps();
      if (g_settings[0] && imu_wrist_raise())
        {
          watchface_wake_display();
        }
      g_stress_value = 20 + imu_motion_energy() * 3 / 4;
      if (g_stress_value > 90)
        {
          g_stress_value = 90;
        }
    }

  /* Keep a deterministic fallback when a board sensor is unavailable. */
  static uint32_t sdu = 0;
  sdu++;
  if (!g_imu_ready && sdu % 10 == 1) {    g_hr_value += (rand() % 7) - 3;  
  if (g_hr_value < 62) g_hr_value = 62;  
  if (g_hr_value > 96) g_hr_value = 96;    g_spo2_value = 96 + (rand() % 4);    g_stress_value += (rand() % 11) - 5;  
  if (g_stress_value < 10) g_stress_value = 10;  
  if (g_stress_value > 90) g_stress_value = 90;
  }
  if (g_mode == MODE_WATCHFACE) {
    face_ui_t *ui = &g_face[g_current];
    char buf[32];
    uint16_t hour_angle = (uint16_t)((t.tm_hour * 300 + t.tm_min * 5) % 3600);
    uint16_t minute_angle = (uint16_t)((t.tm_min * 60 + t.tm_sec) % 3600);
    uint16_t second_angle = (uint16_t)((t.tm_sec * 60) % 3600);

    if (ui->hour != NULL) {
      lv_img_set_angle(ui->hour, hour_angle);
    }
    if (ui->min != NULL) {
      lv_img_set_angle(ui->min, minute_angle);
    }
    if (ui->sec != NULL) {
      lv_img_set_angle(ui->sec, second_angle);
    }
    if (ui->mickey_hour != NULL) {
      lv_img_set_angle(ui->mickey_hour,
                       (uint16_t)((hour_angle + 900) % 3600));
    }
    if (ui->mickey_minute != NULL) {
      lv_img_set_angle(ui->mickey_minute,
                       (uint16_t)((minute_angle + 1800) % 3600));
    }
    if (ui->sport_hour != NULL) {
      lv_img_set_angle(ui->sport_hour, hour_angle);
    }
    if (ui->sport_minute != NULL) {
      lv_img_set_angle(ui->sport_minute, minute_angle);
    }
    if (ui->sport_second != NULL) {
      lv_img_set_angle(ui->sport_second, second_angle);
    }

    if (g_current == FACE_DIGITAL) {
      lv_image_set_src(ui->digits[0], g_digits[t.tm_hour / 10]);
      lv_image_set_src(ui->digits[1], g_digits[t.tm_hour % 10]);
      lv_image_set_src(ui->digits[2], g_digits[t.tm_min / 10]);
      lv_image_set_src(ui->digits[3], g_digits[t.tm_min % 10]);
      snprintf(buf, sizeof(buf), "%04d/%02d/%02d", t.tm_year + 1900,
               t.tm_mon + 1, t.tm_mday);
      lv_label_set_text(ui->lbl_date, buf);
      static const char *wd[] = {"Sun", "Mon", "Tue", "Wed", "Thu",
                                 "Fri", "Sat"};
      lv_label_set_text(ui->lbl_week, wd[t.tm_wday % 7]);
    } else if (g_current == FACE_MICKEY && ui->lbl_time != NULL) {
      snprintf(buf, sizeof(buf), "%02d:%02d", t.tm_hour, t.tm_min);
      lv_label_set_text(ui->lbl_time, buf);
    } else if (g_current == FACE_SPORT && ui->lbl_steps != NULL) {
      snprintf(buf, sizeof(buf), "%d steps", g_steps_value);
      lv_label_set_text(ui->lbl_steps, buf);
    }
  } else if (g_mode == MODE_HEALTH) {  
  char buf[32];    snprintf(buf, sizeof(buf), "%02d:%02d", t.tm_hour, t.tm_min);  
  lv_label_set_text(g_health.lbl_time, buf);    
static const char *wd[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};    snprintf(buf, sizeof(buf), "%02d/%02d %s", t.tm_mon + 1, t.tm_mday, wd[t.tm_wday]);  
  lv_label_set_text(g_health.lbl_date, buf);    update_health_display();
  } else if (g_mode == MODE_GAME && g_game.running) {    uint32_t now_ms = get_tick_ms();  
  if (now_ms - g_game.last_spawn >= GAME_SPAWN_MS) {      game_spawn_note();      g_game.last_spawn = now_ms;  
  }    game_update_notes();
  } else if (g_mode == MODE_APP) {
    app_tick(get_tick_ms());
  }

  if (!g_display_sleeping && g_disp != NULL &&
      lv_display_get_inactive_time(g_disp) >= WATCHFACE_IDLE_TIMEOUT_MS)
    {
      watchface_sleep_display();
    }

  {
    uint32_t tick_elapsed_ms = get_tick_ms() - tick_start_ms;
    g_face_tick_count++;
    g_face_tick_elapsed_ms += tick_elapsed_ms;
    if (g_face_stats_last_report == 0) {
      g_face_stats_last_report = tick_start_ms;
    }
    if (get_tick_ms() - g_face_stats_last_report >= FACE_STATS_REPORT_MS) {
      uint32_t count = g_face_tick_count;
      uint32_t elapsed_ms = g_face_tick_elapsed_ms;
      printf("watchface: face tick count=%lu avg=%lums total=%lums\n",
             (unsigned long)count,
             (unsigned long)(count != 0 ? elapsed_ms / count : 0),
             (unsigned long)elapsed_ms);
      g_face_tick_count = 0;
      g_face_tick_elapsed_ms = 0;
      g_face_stats_last_report = get_tick_ms();
    }
  }
}
/* Main entry point */
int watchface_main(int argc, char *argv[])
{
  lv_nuttx_dsc_t info;
  lv_nuttx_result_t result;
  lv_timer_t *ticker;

  LV_UNUSED(argc);
  LV_UNUSED(argv);
  lv_init();
  lv_nuttx_dsc_init(&info);

  /* LV_USE_NUTTX_LCD uses the LCD character device. */
  info.fb_path = "/dev/lcd0";
  lv_nuttx_init(&info, &result);
  g_disp = result.disp;
  if (g_disp == NULL)
    {
      printf("Error: display not found\n");
      return EXIT_FAILURE;
    }

  g_w = lv_display_get_horizontal_resolution(g_disp);
  g_h = lv_display_get_vertical_resolution(g_disp);
  printf("Display: %dx%d\n", g_w, g_h);

  g_hr_value = 72;
  g_spo2_value = 98;
  g_stress_value = 45;
  g_steps_value = 0;
  g_battery_value = 85;
  g_power_charging = false;
  g_display_sleeping = false;

  watch_board_init();
  (void)watch_power_init();
  (void)watch_haptics_init();
  watch_haptics_set_enabled(g_settings[1]);
  g_imu_ready = imu_init() == 0;
  g_steps_value = (int)imu_steps();

  build_dial_face(&g_face[0]);
  build_mickey_face(&g_face[1]);
  build_digital_face(&g_face[2]);
  build_sport_face(&g_face[3]);
  build_health_enhanced();
  build_menu();
  build_rhythm_app();
  build_sport_app();
  build_timer_app();
  build_settings_app();
  build_weather_app();
  build_steps_app();
  build_alarm_app();
  build_selector();
  build_system();
  build_boot();
  buttons_init(app_button_cb);

  ticker = lv_timer_create(face_tick, FACE_TICK_PERIOD_MS, NULL);
  lv_timer_ready(ticker);
  lv_screen_load(g_boot_scr);

  ticker = lv_timer_create(boot_done_cb, 1200, NULL);
  lv_timer_set_repeat_count(ticker, 1);

  /* Retry the first full redraw over the LCD driver's async init window. */
  ticker = lv_timer_create(lcd_repaint_cb, 500, NULL);
  lv_timer_set_repeat_count(ticker, 8);

  while (true)
    {
      uint32_t idle = lv_timer_handler();
      usleep(idle * 1000u);
    }

  return EXIT_SUCCESS;
}
