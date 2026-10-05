#!/usr/bin/env python3
"""
build_assets.py
Compiles source PNG sheets in assets/ into GBA 4bpp tile arrays and 15-bit BGR555 palettes.
Generates:
  - include/asset_data.h
  - src/asset_data.c
"""

import os
from PIL import Image

def rgb_to_gba(r, g, b):
    # 5 bits per channel (0..31)
    r5 = (r >> 3) & 0x1F
    g5 = (g >> 3) & 0x1F
    b5 = (b >> 3) & 0x1F
    return r5 | (g5 << 5) | (b5 << 10)

def extract_palette(img):
    pal_data = img.getpalette()
    gba_pal = []
    for i in range(16):
        if pal_data and (i * 3 + 2) < len(pal_data):
            r = pal_data[i * 3 + 0]
            g = pal_data[i * 3 + 1]
            b = pal_data[i * 3 + 2]
            gba_pal.append(rgb_to_gba(r, g, b))
        else:
            gba_pal.append(0)
    return gba_pal

def slice_8x8_tile(img, px, py):
    """
    Slices an 8x8 tile at pixel (px, py) and converts to 32 bytes of 4bpp GBA data.
    Row by row: 4 bytes per row.
    Byte: (pixel_even & 0x0F) | ((pixel_odd & 0x0F) << 4)
    """
    tile_bytes = []
    for y in range(8):
        row_pixels = []
        for x in range(8):
            p = img.getpixel((px + x, py + y))
            row_pixels.append(p & 0x0F)
        # Pack 8 pixels into 4 bytes
        b0 = (row_pixels[0] & 0x0F) | ((row_pixels[1] & 0x0F) << 4)
        b1 = (row_pixels[2] & 0x0F) | ((row_pixels[3] & 0x0F) << 4)
        b2 = (row_pixels[4] & 0x0F) | ((row_pixels[5] & 0x0F) << 4)
        b3 = (row_pixels[6] & 0x0F) | ((row_pixels[7] & 0x0F) << 4)
        tile_bytes.extend([b0, b1, b2, b3])
    return tile_bytes

def slice_16x16_sprite(img, sx, sy):
    """
    Slices a 16x16 sprite into 4 8x8 tiles in 1D mapping order:
    TL (0..7, 0..7), TR (8..15, 0..7), BL (0..7, 8..15), BR (8..15, 8..15)
    Returns list of 4 tiles (each 32 bytes).
    """
    tiles = []
    tiles.append(slice_8x8_tile(img, sx, sy))
    tiles.append(slice_8x8_tile(img, sx + 8, sy))
    tiles.append(slice_8x8_tile(img, sx, sy + 8))
    tiles.append(slice_8x8_tile(img, sx + 8, sy + 8))
    return tiles

def format_c_array_u8(name, data_bytes, bytes_per_line=16):
    lines = []
    lines.append(f"const u8 {name}[{len(data_bytes)}] __attribute__((aligned(4))) = {{")
    for i in range(0, len(data_bytes), bytes_per_line):
        chunk = data_bytes[i:i + bytes_per_line]
        hex_str = ", ".join(f"0x{b:02X}" for b in chunk)
        lines.append(f"    {hex_str},")
    lines.append("};\n")
    return "\n".join(lines)

def format_c_array_u16(name, data_u16):
    lines = []
    lines.append(f"const u16 {name}[{len(data_u16)}] = {{")
    hex_str = ", ".join(f"0x{val:04X}" for val in data_u16)
    lines.append(f"    {hex_str}")
    lines.append("};\n")
    return "\n".join(lines)

def main():
    print("Compiling Gameloft graphic assets to C sources...")
    
    # 1. Load images
    img_angkor = Image.open("assets/tiles_angkor.png")
    img_bavaria = Image.open("assets/tiles_bavaria.png")
    img_explorer = Image.open("assets/sprites_explorer.png")
    img_enemies = Image.open("assets/sprites_enemies.png")
    img_objects = Image.open("assets/sprites_objects.png")
    img_ui = Image.open("assets/ui_title.png")

    # 2. Extract Palettes
    pal_angkor = extract_palette(img_angkor)
    pal_bavaria = extract_palette(img_bavaria)
    pal_explorer = extract_palette(img_explorer)
    pal_enemies = extract_palette(img_enemies)
    pal_objects = extract_palette(img_objects)
    pal_ui = extract_palette(img_ui)

    # 3. Slice Metatiles for Angkor Wat (Row 0: 16 metatiles, Row 1: 16 metatiles = 32 metatiles = 128 8x8 tiles)
    tiles_angkor_bytes = []
    for row in range(2):
        for col in range(16):
            mx = col * 16
            my = row * 16
            tiles_angkor_bytes.extend(slice_8x8_tile(img_angkor, mx, my))
            tiles_angkor_bytes.extend(slice_8x8_tile(img_angkor, mx + 8, my))
            tiles_angkor_bytes.extend(slice_8x8_tile(img_angkor, mx, my + 8))
            tiles_angkor_bytes.extend(slice_8x8_tile(img_angkor, mx + 8, my + 8))

    # 4. Slice Metatiles for Bavaria (32 metatiles = 128 8x8 tiles)
    tiles_bavaria_bytes = []
    for row in range(2):
        for col in range(16):
            mx = col * 16
            my = row * 16
            tiles_bavaria_bytes.extend(slice_8x8_tile(img_bavaria, mx, my))
            tiles_bavaria_bytes.extend(slice_8x8_tile(img_bavaria, mx + 8, my))
            tiles_bavaria_bytes.extend(slice_8x8_tile(img_bavaria, mx, my + 8))
            tiles_bavaria_bytes.extend(slice_8x8_tile(img_bavaria, mx + 8, my + 8))

    # 5. Slice Explorer Sprites (16 sprites of 16x16 = 64 8x8 tiles)
    sprites_explorer_bytes = []
    for i in range(16):
        sx = i * 16
        for t in slice_16x16_sprite(img_explorer, sx, 0):
            sprites_explorer_bytes.extend(t)

    # 6. Slice Enemy Sprites (8 sprites of 16x16 = 32 8x8 tiles)
    sprites_enemies_bytes = []
    for i in range(8):
        sx = i * 16
        for t in slice_16x16_sprite(img_enemies, sx, 0):
            sprites_enemies_bytes.extend(t)

    # 7. Slice Object & FX Sprites (16 sprites of 16x16 = 64 8x8 tiles)
    sprites_objects_bytes = []
    for i in range(16):
        sx = i * 16
        for t in slice_16x16_sprite(img_objects, sx, 0):
            sprites_objects_bytes.extend(t)

    # 8. Slice UI tiles (14 8x8 tiles: Diamond, Hearts, Key, Star, Box Frame)
    tiles_ui_bytes = []
    for i in range(14):
        tx = i * 8
        tiles_ui_bytes.extend(slice_8x8_tile(img_ui, tx, 0))

    # 9. Slice Title Logo tiles (20 tiles wide x 4 tiles high = 80 tiles)
    tiles_title_logo_bytes = []
    for ty in range(4):
        for tx in range(20):
            tiles_title_logo_bytes.extend(slice_8x8_tile(img_ui, tx * 8, 16 + (ty * 8)))

    # Write include/asset_data.h
    with open("include/asset_data.h", "w") as f:
        f.write("""#ifndef ASSET_DATA_H
#define ASSET_DATA_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Palettes
extern const u16 g_pal_bg_ui[16];
extern const u16 g_pal_bg_angkor[16];
extern const u16 g_pal_bg_bavaria[16];
extern const u16 g_pal_obj_explorer[16];
extern const u16 g_pal_obj_enemies[16];
extern const u16 g_pal_obj_objects[16];

// Tile Data (4bpp planar, 32 bytes per 8x8 tile)
extern const u8 g_tiles_angkor[4096];     // 128 tiles (32 metatiles * 4)
extern const u8 g_tiles_bavaria[4096];    // 128 tiles
extern const u8 g_sprites_explorer[2048]; // 64 tiles (16 sprites * 4)
extern const u8 g_sprites_enemies[1024];  // 32 tiles (8 sprites * 4)
extern const u8 g_sprites_objects[2048];  // 64 tiles (16 sprites * 4)
extern const u8 g_tiles_ui[448];          // 14 tiles (HUD icons & frames)
extern const u8 g_tiles_title_logo[2560]; // 80 tiles (20x4 title banner)

#ifdef __cplusplus
}
#endif

#endif // ASSET_DATA_H
""")

    # Write src/asset_data.c
    with open("src/asset_data.c", "w") as f:
        f.write("""#include "asset_data.h"

// =========================================================================
// Gameloft Diamond Rush BGR555 Palettes
// =========================================================================

""")
        f.write(format_c_array_u16("g_pal_bg_ui", pal_ui))
        f.write(format_c_array_u16("g_pal_bg_angkor", pal_angkor))
        f.write(format_c_array_u16("g_pal_bg_bavaria", pal_bavaria))
        f.write(format_c_array_u16("g_pal_obj_explorer", pal_explorer))
        f.write(format_c_array_u16("g_pal_obj_enemies", pal_enemies))
        f.write(format_c_array_u16("g_pal_obj_objects", pal_objects))

        f.write("""
// =========================================================================
// 4bpp Tile Data Tables
// =========================================================================

""")
        f.write(format_c_array_u8("g_tiles_angkor", tiles_angkor_bytes))
        f.write(format_c_array_u8("g_tiles_bavaria", tiles_bavaria_bytes))
        f.write(format_c_array_u8("g_sprites_explorer", sprites_explorer_bytes))
        f.write(format_c_array_u8("g_sprites_enemies", sprites_enemies_bytes))
        f.write(format_c_array_u8("g_sprites_objects", sprites_objects_bytes))
        f.write(format_c_array_u8("g_tiles_ui", tiles_ui_bytes))
        f.write(format_c_array_u8("g_tiles_title_logo", tiles_title_logo_bytes))

    print("Successfully built include/asset_data.h and src/asset_data.c!")

if __name__ == "__main__":
    main()
