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


def font_codepoints(font_source):
    unicode_lists = {}
    for match in re.finditer(
        r"static const uint16_t (unicode_list_\d+)\[\] = \{(.*?)\};",
        font_source,
        re.DOTALL,
    ):
        unicode_lists[match.group(1)] = [
            int(value, 0)
            for value in re.findall(r"0x[0-9a-fA-F]+|\b\d+\b", match.group(2))
        ]

    cmap_match = re.search(
        r"static const lv_font_fmt_txt_cmap_t cmaps\[\] =\s*\{(.*?)\n\};",
        font_source,
        re.DOTALL,
    )
    if cmap_match is None:
        raise AssertionError("missing generated font cmap")

    codepoints = set()
    for entry in re.finditer(r"\{(.*?)\}", cmap_match.group(1), re.DOTALL):
        body = entry.group(1)
        range_start = re.search(r"\.range_start\s*=\s*(\d+)", body)
        range_length = re.search(r"\.range_length\s*=\s*(\d+)", body)
        unicode_list = re.search(
            r"\.unicode_list\s*=\s*(unicode_list_\d+|NULL)", body
        )
        if range_start is None or range_length is None or unicode_list is None:
            continue

        start = int(range_start.group(1))
        list_name = unicode_list.group(1)
        if list_name == "NULL":
            codepoints.update(range(start, start + int(range_length.group(1))))
        else:
            codepoints.update(start + offset for offset in unicode_lists[list_name])

    return codepoints


class WatchfaceContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = (SRC / "watchface_main.c").read_text(encoding="utf-8")
        cls.generator = (ROOT / "assets_gen.py").read_text(encoding="utf-8")
        cls.header = (SRC / "wf_assets.h").read_text(encoding="utf-8")
        cls.makefile = (ROOT / "Makefile").read_text(encoding="utf-8")
        cls.cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

    def test_dial_uses_official_background_and_hands(self):
        body = section(self.source, "build_dial_face", "build_mickey_face")
        for asset in (
            "img_official_dial_bg",
            "img_official_dial_hour",
            "img_official_dial_minute",
            "img_official_dial_second",
            "lv_img_set_pivot",
        ):
            self.assertIn(asset, body)

    def test_mickey_uses_character_layers_and_rotating_hands(self):
        body = section(self.source, "build_mickey_face", "build_digital_face")
        for asset in (
            "img_mickey_bg",
            "img_head",
            "img_body",
            "img_eye0",
            "img_mickey_hand_hour",
            "img_mickey_hand_minute",
            "lv_img_set_pivot",
        ):
            self.assertIn(asset, body)

    def test_digital_uses_background_and_colon_layer(self):
        body = section(self.source, "build_digital_face", "build_sport_face")
        self.assertIn("img_digital_bg", body)
        self.assertIn("lbl_colon", self.source)

    def test_sport_uses_official_background_and_hands(self):
        body = section(self.source, "build_sport_face", "build_health_enhanced")
        for asset in (
            "img_sport_bg",
            "img_official_sport_hour",
            "img_official_sport_minute",
            "img_official_sport_second",
            "lv_img_set_pivot",
        ):
            self.assertIn(asset, body)

    def test_face_tick_updates_all_analog_face_hands(self):
        tick = self.source[self.source.index("static void face_tick") :]
        for hand in (
            "ui->hour",
            "ui->min",
            "ui->sec",
            "ui->mickey_hour",
            "ui->mickey_minute",
            "ui->sport_hour",
            "ui->sport_minute",
            "ui->sport_second",
        ):
            self.assertIn(f"lv_img_set_angle({hand}", tick)

    def test_face_tick_normalizes_analog_angles(self):
        tick = self.source[self.source.index("static void face_tick") :]
        self.assertIn("hour_angle + 900", tick)
        self.assertIn("minute_angle + 1800", tick)
        self.assertIn("% 3600", tick)

    def test_digital_row_is_centered_for_current_panel(self):
        body = section(self.source, "build_digital_face", "build_sport_face")
        self.assertIn("total_width", body)
        self.assertIn("colon_width", body)
        self.assertIn("(g_w - total_width) / 2", body)

    def test_generator_maps_sport_and_full_mickey_preview(self):
        self.assertIn(
            '("sport_bg", "clock_rotate_bg_bg.png", "bg", None)',
            self.generator,
        )
        self.assertIn('("prev_mickey",', self.generator)
        self.assertIn("mickey_preview", self.generator)
        self.assertNotIn(
            '("prev_mickey", "clock_mickey_head.png", "bg_small", None)',
            self.generator,
        )

    def test_new_layers_are_declared(self):
        for asset in (
            "img_mickey_hand_hour",
            "img_mickey_hand_minute",
            "img_official_sport_hour",
            "img_official_sport_minute",
            "img_official_sport_second",
        ):
            self.assertIn(f"extern const lv_image_dsc_t {asset};", self.header)

    def test_face_tick_aggregates_periodic_performance_stats(self):
        tick = self.source[self.source.index("static void face_tick") :]
        self.assertIn("FACE_STATS_REPORT_MS", self.source)
        self.assertIn("g_face_tick_count", self.source)
        self.assertIn("g_face_tick_elapsed_ms", self.source)
        self.assertIn("g_face_stats_last_report", tick)
        self.assertIn("printf(\"watchface: face tick", tick)

    def test_face_scheduler_targets_smooth_analog_motion(self):
        self.assertIn("FACE_TICK_PERIOD_MS", self.source)
        self.assertIn("#define FACE_TICK_PERIOD_MS 33", self.source)
        self.assertIn("lv_timer_create(face_tick, FACE_TICK_PERIOD_MS", self.source)
        self.assertNotIn("lv_timer_create(face_tick, 200", self.source)

    def test_menu_animation_avoids_float_distance_work(self):
        self.assertIn("HONEY_MENU_ANIM_PERIOD_MS", self.source)
        self.assertIn("distance_squared", self.source)
        self.assertIn("HONEY_MENU_ANIM_PERIOD_MS, NULL", self.source)
        self.assertNotIn("sqrt((double)dx * dx + (double)dy * dy)", self.source)

    def test_app_refresh_is_throttled_without_delaying_direct_actions(self):
        tick = self.source[self.source.index("static void app_tick(uint32_t now_ms)") :]
        self.assertIn("APP_RENDER_PERIOD_MS", self.source)
        self.assertIn("g_app_last_render_ms", self.source)
        self.assertIn("now_ms - g_app_last_render_ms", tick)
        self.assertIn("app_render()", tick)

    def test_chinese_ui_font_is_built_and_selected(self):
        self.assertIn("font_zh_16", self.source)
        self.assertIn("font_zh_20", self.source)
        self.assertIn("src/font_zh_16.c", self.makefile)
        self.assertIn("src/font_zh_20.c", self.makefile)
        self.assertIn("src/font_zh_16.c", self.cmake)
        self.assertIn("src/font_zh_20.c", self.cmake)

    def test_app_unit_label_uses_chinese_font(self):
        shell = section(self.source, "build_menu_app_shell", "build_rhythm_app")
        self.assertIn(
            "lv_obj_set_style_text_font(g_app.unit, &font_zh_16, 0);",
            shell,
        )

    def test_app_render_uses_30fps_dynamic_refresh_and_cached_updates(self):
        self.assertIn("#define APP_RENDER_PERIOD_MS 33", self.source)
        self.assertIn("label_set_if_changed", self.source)
        self.assertIn("app_set_hidden_if_changed", self.source)
        self.assertIn("g_app_progress_width", self.source)
        self.assertIn("if (page_changed)\n          {\n            lv_obj_align(g_app.value, LV_ALIGN_TOP_MID, 0, 92);", self.source)
        self.assertNotIn("lv_obj_remove_style_all(row);", self.source)

    def test_chinese_fonts_use_uncompressed_bitmaps(self):
        for font_name in ("font_zh_16.c", "font_zh_20.c"):
            font_source = (SRC / font_name).read_text(encoding="utf-8")
            self.assertIn(".bitmap_format = 0", font_source)
            self.assertNotIn(".bitmap_format = 1", font_source)

    def test_chinese_fonts_cover_every_non_ascii_ui_character(self):
        string_literals = re.findall(r'"(?:\\.|[^"\\])*"', self.source)
        required = {
            ord(character)
            for literal in string_literals
            for character in literal
            if ord(character) > 127
        }

        for font_name in ("font_zh_16.c", "font_zh_20.c"):
            font_source = (SRC / font_name).read_text(encoding="utf-8")
            missing = sorted(required - font_codepoints(font_source))
            self.assertFalse(
                missing,
                f"{font_name} missing UI codepoints: "
                + ", ".join(f"U+{codepoint:04X}" for codepoint in missing),
            )

    def test_non_face_pages_use_chinese_labels(self):
        for label in (
            '"经典"',
            '"米奇"',
            '"数字"',
            '"电源"',
            '"重启"',
            '"再次点击确认"',
            '"再次点击重启"',
        ):
            self.assertIn(label, self.source)


if __name__ == "__main__":
    unittest.main()
