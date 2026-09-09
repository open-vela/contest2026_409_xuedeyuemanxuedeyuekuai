#!/usr/bin/env python3
# Generates RGB565 / RGB565A8 binary arrays for the HuangshanPi watchface app.
# Run inside WSL (has Pillow). Sources come from the beatpulse quickapp pack.
import os
import struct

from PIL import Image

OUT = os.path.dirname(os.path.abspath(__file__))
ASSETS = os.path.join(OUT, "assets")

# Keep the generator usable from both the WSL build shell and Windows Python.
# The source pack lives beside this application in the contest workspace.
WORKSPACE = os.path.dirname(OUT)
SRC = os.path.join(WORKSPACE, "beatpulse", "src", "common", "image")
if not os.path.isdir(SRC):
    SRC = "/mnt/c/Users/21561/Desktop/比赛/beatpulse/src/common/image"

SRC_OFFICIAL = os.environ.get(
    "SIFLI_WATCH_IMAGES",
    os.path.join("D:\\OpenSiFli", "yellow_mountain_example", "lvgl", "watch",
                 "src", "resource", "images", "common"),
)
if not os.path.isdir(SRC_OFFICIAL):
    SRC_OFFICIAL = "/mnt/d/OpenSiFli/yellow_mountain_example/lvgl/watch/src/resource/images/common"
W, H = 390, 450
DIGIT_H = 64
PREV_W, PREV_H = 110, 127
S454 = H / 454.0
S480 = H / 480.0
S359 = H / 359.0


def rgb565_bytes(im):
    out = bytearray()
    px = im.load()
    for y in range(im.height):
        for x in range(im.width):
            r, g, b = px[x, y][:3]
            v = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            out += v.to_bytes(2, "little")
    return bytes(out)


def alpha_plane(im):
    im = im.convert("RGBA")
    return bytes(im.getchannel("A").tobytes())


def lv_rle(raw, blk_size=2):
    """Pack bytes as 2-byte blocks using LVGL's tiny RLE stream format."""
    if len(raw) % blk_size:
        raise ValueError("RLE input must be a whole number of blocks")

    blocks = [int.from_bytes(raw[i:i + blk_size], "little")
              for i in range(0, len(raw), blk_size)]
    out = bytearray()
    i = 0
    while i < len(blocks):
        run = 1
        while (i + run < len(blocks) and blocks[i + run] == blocks[i]
               and run < 127):
            run += 1

        if run >= 3:
            out.append(run)
            out += blocks[i].to_bytes(blk_size, "little")
            i += run
        else:
            literal = []
            while i < len(blocks) and len(literal) < 127:
                run = 1
                while (i + run < len(blocks)
                       and blocks[i + run] == blocks[i] and run < 127):
                    run += 1
                if run >= 3:
                    break
                literal.append(blocks[i])
                i += 1
            out.append(0x80 | len(literal))
            for value in literal:
                out += value.to_bytes(blk_size, "little")
    return bytes(out)


def compressed_payload(raw):
    """LVGL compressed variable-image payload (12 byte header + RLE data)."""
    # LVGL RLE always works on 2-byte blocks.  RGB565A8 can have an odd total
    # size; the decoder handles a final half-block and clips it to the original
    # decompressed_size.
    padded = raw if len(raw) % 2 == 0 else raw + b"\0"
    packed = lv_rle(padded)
    return struct.pack("<III", 1, len(packed), len(raw)) + packed


def fit_canvas(src_img):
    """Scale-to-cover 390x450 canvas, centred."""
    img = src_img.convert("RGB")
    s = max(W / img.width, H / img.height)
    nw, nh = max(1, round(img.width * s)), max(1, round(img.height * s))
    rs = img.resize((nw, nh), Image.LANCZOS)
    cv = Image.new("RGB", (W, H), (0, 0, 0))
    cv.paste(rs, ((W - nw) // 2, (H - nh) // 2))
    return cv


def resize_h(src_img, target_h):
    img = src_img.convert("RGBA")
    s = target_h / img.height
    nw = max(1, round(img.width * s))
    return img.resize((nw, target_h), Image.LANCZOS)


def resize_scale(src_img, scale):
    """Scale an asset by the same factor used for its 454/480/359 canvas."""
    img = src_img.convert("RGBA")
    return img.resize((max(1, round(img.width * scale)),
                       max(1, round(img.height * scale))), Image.LANCZOS)


def design_position(x, y):
    """Map a 480x480 historical layout into the fitted 390x450 canvas."""
    return (round((W - H) / 2 + x * S480), round(y * S480))


def build_mickey_preview():
    """Build the selector thumbnail from the same Mickey layers as the face."""
    bg = fit_canvas(Image.open(os.path.join(SRC, "clock_mickey_bg.png"))).convert("RGBA")
    for name, x, y in (
        ("clock_mickey_body.png", 112, 200),
        ("clock_mickey_head.png", 134, 60),
    ):
        layer = resize_scale(Image.open(os.path.join(SRC, name)), S480)
        bg.alpha_composite(layer, dest=design_position(x, y))
    return bg.convert("RGB").resize((PREV_W, PREV_H), Image.LANCZOS)


JOBS = [
    # (symbol, source file, mode, param)
    ("simple_bg", "clock_simple_bg.png", "bg", None),
    ("mickey_bg", "clock_mickey_bg.png", "bg", None),
    ("digital_bg", "clock_digital_bg.png", "bg", None),
    ("sport_bg", "clock_rotate_bg_bg.png", "bg", None),
    ("head", "clock_mickey_head.png", "a8", S480),
    ("body", "clock_mickey_body.png", "a8", S480),
    ("eye0", "clock_mickey_eyes01.png", "a8", S480),
    ("eye1", "clock_mickey_eyes02.png", "a8", S480),
    ("eye2", "clock_mickey_eyes03.png", "a8", S480),
] + [(f"digit{i}", f"digital_b_time_{i}.png", "a8", DIGIT_H) for i in range(10)] + [
    ("prev_simple", "clock_simple_bg.png", "bg_small", None),
    ("prev_mickey", None, "mickey_preview", None),
    ("prev_digital", "clock_digital_bg.png", "bg_small", None),
    ("prev_sport", "clock_rotate_bg_bg.png", "bg_small", None),
]

# The three faces registered by SiFli's official app_clock_main.c are
# rotate_bg, simple and dial.  We port those assets with their original
# hand pivots scaled to the 390x450 HuangshanPi panel.
OFFICIAL_IMAGES = SRC_OFFICIAL + "/large_ezip"
OFFICIAL_HANDS = SRC_OFFICIAL + "/no_ezip"
JOBS += [
    ("official_dial_bg", OFFICIAL_IMAGES + "/img_dial_bg.png", "bg", None),
    ("official_rotate_bg", OFFICIAL_HANDS + "/clock_rotate_bg_bg.png", "bg", None),
    ("official_simple_hour", OFFICIAL_HANDS + "/clock_simple_hour_hand.png", "a8", S454),
    ("official_simple_minute", OFFICIAL_HANDS + "/clock_simple_minute_hand.png", "a8", S454),
    ("official_simple_second", OFFICIAL_HANDS + "/clock_simple_second_hand.png", "a8", S454),
    ("official_dial_hour", OFFICIAL_HANDS + "/clock_dial_point_h.png", "a8", S480),
    ("official_dial_minute", OFFICIAL_HANDS + "/clock_dial_point_m.png", "a8", S480),
    ("official_dial_second", OFFICIAL_HANDS + "/clock_dial_point_s.png", "a8", S480),
    ("official_sport_hour", OFFICIAL_HANDS + "/clock_rotate_bg_hour_hand.png", "a8", S359),
    ("official_sport_minute", OFFICIAL_HANDS + "/clock_rotate_bg_minute_hand.png", "a8", S359),
    ("official_sport_second", OFFICIAL_HANDS + "/fashion_sec.png", "a8", S359),
    ("mickey_hand_hour", OFFICIAL_HANDS + "/clock_mickey_hand_hour.png", "a8", S454),
    ("mickey_hand_minute", OFFICIAL_HANDS + "/clock_mickey_hand_minute.png", "a8", S454),
    ("prev_dial", OFFICIAL_IMAGES + "/img_dial_bg.png", "bg_small", None),
    ("prev_rotate", OFFICIAL_HANDS + "/clock_rotate_bg_bg.png", "bg_small", None),
]

# UI beautification icons pulled straight from the beatpulse quickapp pack.
# They are white-on-transparent so we keep them as RGB565A8 (alpha channel
# drives the tint; LVGL lets us recolor with lv_image_set_style_img_recolor).
JOBS += [
    # Status bar (classic face)
    ("ui_bluetooth",  "bluetooth.png",   "a8", 28),
    ("ui_sunny",      "wt_sunny.png",    "a8", 28),
    # Health page icons
    ("ui_heart",      "img_red_heart.png", "a8", 56),
    ("ui_water",      "ms_water.png",      "a8", 56),
    ("ui_activity",   "img_activity.png",  "a8", 56),
    ("ui_eco",        "eco.png",           "a8", 56),
    ("ui_stress",     "barometer.png",     "a8", 56),
    # Menu grid icons
    ("menu_clock",    "img_clock.png",    "a8", 44),
    ("menu_health",   "img_red_heart.png", "a8", 44),
    ("menu_rhythm",   "img_workout.png",  "a8", 44),
    ("menu_settings", "img_settings.png", "a8", 44),
    ("menu_timer",    "img_stopwatch.png", "a8", 44),
    ("menu_alarm",    "img_alarm.png",     "a8", 44),
    ("menu_weather",  "wt_sunny.png",      "a8", 44),
]


def main():
    os.makedirs(ASSETS, exist_ok=True)
    meta = []
    for sym, fname, mode, param in JOBS:
        if mode == "mickey_preview":
            data_img = build_mickey_preview()
            data = rgb565_bytes(data_img)
            cf, w, h = 0x12, PREV_W, PREV_H
        else:
            img = Image.open(os.path.join(SRC, fname))
            data_img = None
        if mode == "bg":
            conv = fit_canvas(img)
            data = rgb565_bytes(conv)
            cf, w, h = 0x12, W, H          # LV_COLOR_FORMAT_RGB565 (0x12 in v9.2? verified below)
        elif mode == "bg_small":
            cv = fit_canvas(img)
            small = cv.resize((PREV_W, PREV_H), Image.LANCZOS)
            data = rgb565_bytes(small)
            cf, w, h = 0x12, PREV_W, PREV_H
        elif mode != "mickey_preview":
            if param is None:
                rgba = img.convert("RGBA")
            elif isinstance(param, float):
                rgba = resize_scale(img, param)
            else:
                rgba = resize_h(img, param)
            w, h = rgba.size
            plane = rgb565_bytes(rgba) + alpha_plane(rgba)
            data = plane
            cf = 0x14                       # LV_COLOR_FORMAT_RGB565A8
        path = os.path.join(ASSETS, sym + ".bin")
        with open(path, "wb") as f:
            f.write(data)                  # keep the raw preview/inspection asset
        packed = compressed_payload(data)
        path = os.path.join(ASSETS, sym + ".rle.bin")
        with open(path, "wb") as f:
            f.write(packed)
        meta.append((sym, cf, w, h, len(packed), len(data)))
        print(f"{sym:14s} {w}x{h} cf={cf:#04x} "
              f"{len(data)/1024:7.1f} KB -> {len(packed)/1024:7.1f} KB")

    # ---- generated C with byte arrays + descriptors -------------------
    lines = [
        "/* Auto-generated by assets_gen.py -- do not edit */",
        "#include \"lvgl.h\"",
        '#include "../src/wf_assets.h"',
        "",
    ]
    dsc_lines = []
    for sym, cf, w, h, size, raw_size in meta:
        lines.append(
            f"static const uint8_t {sym}_data[{size}] "
            f"__attribute__((aligned(4))) = {{"
        )
        raw = open(os.path.join(ASSETS, sym + ".rle.bin"), "rb").read()
        for i in range(0, len(raw), 16):
            chunk = ",".join(f"0x{b:02x}" for b in raw[i:i + 16])
            lines.append("  " + chunk + ",")
        lines.append("};")
        lines.append("")
        fmt_macro = "LV_COLOR_FORMAT_RGB565A8" if cf == 0x14 else "LV_COLOR_FORMAT_RGB565"
        dsc_lines.append(
            f"const lv_image_dsc_t img_{sym} = {{\n"
            f"  .header = {{\n"
            f"    .magic = LV_IMAGE_HEADER_MAGIC,\n"
            f"    .cf = {fmt_macro},\n"
            f"    .flags = LV_IMAGE_FLAGS_COMPRESSED,\n"
            f"    .w = {w},\n"
            f"    .h = {h},\n"
            f"    .stride = {w * 2},\n"
            f"  }},\n"
            f"  .data_size = sizeof({sym}_data),\n"
            f"  .data = {sym}_data,\n"
            f"}};"
        )
    lines.extend(dsc_lines)
    with open(os.path.join(OUT, "assets", "wf_data.c"), "w") as f:
        f.write("\n".join(lines) + "\n")
    print("generated", os.path.join(OUT, "assets", "wf_data.c"))


main()
