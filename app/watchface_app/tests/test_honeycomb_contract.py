import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
SRC = ROOT / "src"


class HoneycombContractTests(unittest.TestCase):
    def test_model_exposes_layout_and_snap_operations(self):
        header = (SRC / "honeycomb_model.h").read_text(encoding="utf-8")
        self.assertIn("honeycomb_cell_point", header)
        self.assertIn("honeycomb_nearest_cell", header)
        self.assertIn("honeycomb_snap_offset", header)

    def test_menu_declares_real_asset_grid_and_gesture_states(self):
        source = (SRC / "watchface_main.c").read_text(encoding="utf-8")
        self.assertIn("HONEY_MENU_ITEMS", source)
        self.assertIn("LV_EVENT_PRESSING", source)
        self.assertIn("LV_EVENT_RELEASED", source)
        self.assertIn("img_menu_health", source)
        self.assertIn("img_menu_rhythm", source)
        self.assertNotIn("img_menu_nes", source)

    def test_asset_generator_registers_non_placeholder_menu_icons(self):
        generator = (ROOT / "assets_gen.py").read_text(encoding="utf-8")
        for name in (
            "menu_health",
            "menu_rhythm",
            "menu_settings",
            "menu_timer",
            "menu_alarm",
            "menu_weather",
        ):
            self.assertIn(f'("{name}"', generator)
        self.assertNotIn('("menu_nes"', generator)

    def test_menu_uses_declared_animation_timer(self):
        source = (SRC / "watchface_main.c").read_text(encoding="utf-8")
        self.assertIn("g_menu_anim = lv_timer_create", source)
        self.assertNotIn("g_honey_anim", source)

    def test_watchface_sources_match_nuttx_lvgl_build_contract(self):
        model = (SRC / "honeycomb_model.c").read_text(encoding="utf-8")
        source = (SRC / "watchface_main.c").read_text(encoding="utf-8")
        self.assertIn("#include <stddef.h>", model)
        self.assertNotIn("lv_font_montserrat_12", source)
        self.assertIn("lv_font_montserrat_16", source)

    def test_nes_integration_is_removed_from_watchface_tree(self):
        self.assertFalse((ROOT / "nes").exists())
        self.assertFalse((ROOT.parent / "rom.nes").exists())

        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        makefile = (ROOT / "Makefile").read_text(encoding="utf-8")
        self.assertNotIn("nes", cmake.lower())
        self.assertNotIn("nes", makefile.lower())

        assets = (ROOT / "assets" / "wf_data.c").read_text(encoding="utf-8")
        header = (SRC / "wf_assets.h").read_text(encoding="utf-8")
        self.assertNotIn("menu_nes", assets)
        self.assertNotIn("img_menu_nes", header)

    def test_apple_watch_face_edit_flow_is_declared(self):
        source = (SRC / "watchface_main.c").read_text(encoding="utf-8")
        self.assertIn("MODE_SELECTOR", source)
        self.assertIn("MODE_SYSTEM", source)
        self.assertIn("LV_EVENT_LONG_PRESSED", source)
        self.assertIn("selector_gesture_cb", source)
        self.assertIn("g_selector_face", source)
        self.assertIn("g_selector_preview", source)
        self.assertIn("switch_to_face(g_selector_face)", source)
        self.assertIn("g_face[g_current]", source)

    def test_button_policy_uses_confirmation_and_combo_reset(self):
        app_buttons = (SRC / "buttons.c").read_text(encoding="utf-8")
        board_buttons = (SRC / "sf32lb52_buttons.c").read_text(encoding="utf-8")
        self.assertIn("PRESS_LONG", app_buttons)
        self.assertIn("LONG_PRESS_MS", app_buttons)
        self.assertNotIn("up_systemreset", app_buttons)
        self.assertIn("SF32LB52_BUTTON_KEY1_BIT | SF32LB52_BUTTON_KEY2_BIT", board_buttons)
        self.assertIn("SF32LB52_BUTTON_COMBO_RESET_MS", board_buttons)
        self.assertIn("up_systemreset", board_buttons)
        self.assertNotIn("SF32LB52_BUTTON_LONG_RESET_MS", board_buttons)

    def test_build_script_syncs_board_button_policy(self):
        script = (ROOT.parent.parent / "build_watchface.sh").read_text(encoding="utf-8")
        self.assertIn("sf32lb52_buttons.c", script)
        self.assertIn("$APP_SRC/src/sf32lb52_buttons.c", script)


if __name__ == "__main__":
    unittest.main()
