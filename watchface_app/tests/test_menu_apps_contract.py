import pathlib
import re
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
SRC = ROOT / "src"


def section(source, name, next_name):
    pattern = rf"static void {name}\(.*?\n(?=static void {next_name}\()"
    match = re.search(pattern, source, re.DOTALL)
    if match is None:
        raise AssertionError(f"missing source section: {name}")
    return match.group(0)


class MenuAppsContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = (SRC / "watchface_main.c").read_text(encoding="utf-8")

    def test_menu_has_a_real_screen_for_every_visible_entry(self):
        for app in (
            "MENU_APP_HEALTH",
            "MENU_APP_RHYTHM",
            "MENU_APP_SPORT",
            "MENU_APP_TIMER",
            "MENU_APP_SETTINGS",
            "MENU_APP_WEATHER",
            "MENU_APP_STEPS",
            "MENU_APP_ALARM",
        ):
            self.assertIn(app, self.source)

        for builder in (
            "build_health_enhanced",
            "build_rhythm_app",
            "build_sport_app",
            "build_timer_app",
            "build_settings_app",
            "build_weather_app",
            "build_steps_app",
            "build_alarm_app",
        ):
            self.assertIn(builder, self.source)

    def test_menu_dispatches_to_app_screens_instead_of_placeholders(self):
        body = section(self.source, "honey_menu_open", "build_menu")
        self.assertIn("open_menu_app(index)", body)
        self.assertNotIn("Rhythm placeholder", body)
        self.assertNotIn("reuses the game screen", body)
        self.assertNotIn("switch_to_face(g_current);", body)

    def test_app_shell_supports_back_and_touch_navigation(self):
        self.assertIn("MODE_APP", self.source)
        self.assertIn("g_app_id", self.source)
        self.assertIn("app_screen_event_cb", self.source)
        self.assertIn("return_to_menu", self.source)
        self.assertIn("LV_EVENT_GESTURE", self.source)

    def test_timer_has_independent_running_and_reset_state(self):
        for token in (
            "g_timer_running",
            "g_timer_elapsed_ms",
            "g_timer_started_ms",
            "timer_toggle_cb",
            "timer_reset_cb",
        ):
            self.assertIn(token, self.source)

    def test_timer_supports_stopwatch_and_countdown_modes(self):
        for token in (
            "TIMER_MODE_STOPWATCH",
            "TIMER_MODE_COUNTDOWN",
            "g_timer_mode",
            "g_timer_countdown_ms",
            "g_timer_countdown_running",
            "g_timer_countdown_started_ms",
            "timer_countdown_update",
            "timer_set_mode",
        ):
            self.assertIn(token, self.source)

    def test_countdown_has_expiry_feedback_and_progress(self):
        for token in (
            "TIMER_DEFAULT_MS",
            "g_timer_countdown_expired",
            "时间到",
            "TIMER_MODE_COUNTDOWN",
            "g_app.progress_bg",
        ):
            self.assertIn(token, self.source)

    def test_timer_mode_uses_segmented_touch_controls(self):
        for token in (
            "timer_mode_bar",
            "timer_stopwatch_mode",
            "timer_countdown_mode",
            "timer_set_mode(TIMER_MODE_STOPWATCH)",
            "timer_set_mode(TIMER_MODE_COUNTDOWN)",
        ):
            self.assertIn(token, self.source)

    def test_rhythm_preserves_sub_beat_time_between_ticks(self):
        self.assertIn("g_rhythm_remainder", self.source)
        self.assertIn("rhythm_update(uint32_t now_ms)", self.source)
        self.assertIn("units / 60000", self.source)

    def test_sport_summary_uses_live_elapsed_time(self):
        self.assertIn("uint32_t sport_elapsed_ms", self.source)
        self.assertIn("sport_elapsed_ms / 60000", self.source)

    def test_alarm_test_has_visible_timed_feedback(self):
        for token in (
            "g_alarm_test_active",
            "g_alarm_test_until_ms",
            "闹钟测试：响铃",
        ):
            self.assertIn(token, self.source)

    def test_sensor_values_are_polled_and_reused_by_health_apps(self):
        self.assertIn("imu_init()", self.source)
        self.assertIn("imu_poll()", self.source)
        self.assertIn("imu_steps()", self.source)
        self.assertIn("imu_motion_energy()", self.source)

    def test_app_layout_separates_summary_from_data_rows(self):
        self.assertIn("APP_ROWS_TOP_DEFAULT", self.source)
        self.assertIn("APP_ROWS_TOP_HEALTH", self.source)
        self.assertIn("APP_DETAIL_TOP_DEFAULT", self.source)
        self.assertIn("APP_DETAIL_TOP_SETTINGS", self.source)
        self.assertIn(
            "g_app_id == MENU_APP_HEALTH ? APP_ROWS_TOP_HEALTH : APP_ROWS_TOP_DEFAULT",
            self.source,
        )
        self.assertIn(
            "lv_obj_align(g_app.detail, LV_ALIGN_TOP_MID, 0, APP_DETAIL_TOP_DEFAULT)",
            self.source,
        )
        self.assertIn(
            "lv_obj_align(g_app.detail, LV_ALIGN_TOP_MID, 0, APP_DETAIL_TOP_SETTINGS)",
            self.source,
        )

    def test_key1_is_back_inside_menu_and_apps(self):
        callback = section(self.source, "app_button_cb", "face_tick")
        self.assertIn("return_to_watchface()", callback)
        self.assertIn("return_to_menu()", callback)
        self.assertNotIn("honey_menu_open(g_honey_focus)", callback)


if __name__ == "__main__":
    unittest.main()
