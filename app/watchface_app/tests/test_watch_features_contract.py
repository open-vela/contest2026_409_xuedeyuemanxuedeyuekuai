import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
SRC = ROOT / "src"


def text(path):
    if not path.exists():
        return ""
    return path.read_text(encoding="utf-8")


class WatchFeaturesContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.main = text(SRC / "watchface_main.c")
        cls.imu = text(SRC / "sensor_imu.c")
        cls.imu_header = text(SRC / "sensor_imu.h")
        cls.power = text(SRC / "watch_power.c")
        cls.power_header = text(SRC / "watch_power.h")
        cls.haptics = text(SRC / "watch_haptics.c")
        cls.haptics_header = text(SRC / "watch_haptics.h")
        cls.board = text(SRC / "watch_board.c")
        cls.board_policy = text(SRC / "sf32lb52_buttons.c")
        cls.makefile = text(ROOT / "Makefile")
        cls.cmake = text(ROOT / "CMakeLists.txt")

    def test_power_module_exposes_a_safe_snapshot_api(self):
        for token in (
            "watch_power_snapshot_t",
            "watch_power_init",
            "watch_power_poll",
            "watch_power_get",
            "watch_power_low_battery_event",
            "battery_mv",
            "battery_percent",
            "vbus_present",
            "charging",
        ):
            self.assertIn(token, self.power_header)

    def test_power_reads_the_board_adc_and_clamps_battery_state(self):
        for token in (
            '"/dev/adc0"',
            "struct adc_msg_s",
            "ANIOC_TRIGGER",
            "ADC_CHAN_VBAT",
            "am_channel",
            "am_data",
            "WATCH_POWER_LOW_BATTERY_PERCENT",
            "watch_board_vbus_present",
            "WATCH_POWER_SAMPLE_PERIOD_MS",
        ):
            self.assertIn(token, self.power)
        self.assertIn("battery_percent < 0", self.power)
        self.assertIn("battery_percent > 100", self.power)

    def test_vibration_service_uses_nonblocking_pwm_and_has_a_stop_path(self):
        for token in (
            "watch_haptics_init",
            "watch_haptics_poll",
            "watch_haptics_pulse",
            "watch_haptics_set_enabled",
            '"/dev/pwm0"',
            "O_NONBLOCK",
            "PWMIOC_SETCHARACTERISTICS",
            "PWMIOC_START",
            "PWMIOC_STOP",
            "frequency",
            "duty",
        ):
            self.assertIn(token, self.haptics)
        self.assertIn("enabled", self.haptics_header)

    def test_board_glue_owns_vbus_input_and_vibrator_pinmux(self):
        for token in ("watch_board_init", "watch_board_vbus_present"):
            self.assertIn(token, self.board_policy)
        for token in (
            "GET_PIN_2(hwp_gpio1, 44)",
            "sifli_gpio_read",
            "PAD_PA20",
            "PA20_TIM",
            "HAL_PIN_Set",
        ):
            self.assertIn(token, self.board_policy)

    def test_imu_exposes_persistent_steps_and_wrist_raise_detection(self):
        for token in (
            "imu_steps_load",
            "imu_steps_save",
            "imu_wrist_raise",
            "/data/watch_steps.dat",
            "rename(",
            "crc",
            "WRIST_RAISE_COOLDOWN_MS",
        ):
            self.assertIn(token, self.imu)
        for token in (
            "imu_steps_load",
            "imu_steps_save",
            "imu_wrist_raise",
        ):
            self.assertIn(token, self.imu_header)

    def test_main_integrates_power_haptics_idle_sleep_and_motion_wake(self):
        for token in (
            "#include <sys/ioctl.h>",
            '#include "watch_power.h"',
            '#include "watch_haptics.h"',
            '#include "sensor_imu.h"',
            "LCDDEVIO_SETPOWER",
            "lv_display_get_inactive_time",
            "lv_display_trigger_activity",
            "WATCHFACE_IDLE_TIMEOUT_MS",
            "watch_power_poll",
            "watch_haptics_poll",
            "watch_haptics_pulse",
            "imu_wrist_raise",
            "watch_power_low_battery_event",
        ):
            self.assertIn(token, self.main)

    def test_steps_do_not_use_a_fake_fixed_offset(self):
        self.assertNotIn("1234 + imu_steps()", self.main)
        self.assertNotIn("g_steps_value = 1234", self.main)
        self.assertNotIn('lv_label_set_text(ui->lbl_steps, "1234', self.main)

    def test_build_lists_include_the_new_runtime_modules(self):
        for source in (
            "src/watch_power.c",
            "src/watch_haptics.c",
            "src/watch_board.c",
            "src/sensor_imu.c",
        ):
            self.assertIn(source, self.makefile)
            self.assertIn(source, self.cmake)


if __name__ == "__main__":
    unittest.main()
