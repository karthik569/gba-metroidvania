#!/usr/bin/env python3
"""
generate_source_images.py
Generates authentic Gameloft-grade pixel art PNG sheets for Diamond Rush GBA.
Creates:
  - assets/tiles_angkor.png
  - assets/tiles_bavaria.png
  - assets/sprites_explorer.png
  - assets/sprites_enemies.png
  - assets/sprites_objects.png
  - assets/ui_title.png
"""

import os
from PIL import Image, ImageDraw

os.makedirs("assets", exist_ok=True)

# Helper to create an indexed PNG from pixel rows with an exact 16-color palette
def create_indexed_image(width, height, palette_rgb16, pixel_data):
    """
    palette_rgb16: list of 16 (r, g, b) tuples. Index 0 is transparent / background.
    pixel_data: 2D array [height][width] with values 0..15.
    """
    img = Image.new("P", (width, height), 0)
    flat_palette = []
    for c in palette_rgb16:
        flat_palette.extend([c[0], c[1], c[2]])
    # Pad to 256 colors for PIL palette
    while len(flat_palette) < 768:
        flat_palette.extend([0, 0, 0])
    img.putpalette(flat_palette)

    flat_pixels = []
    for y in range(height):
        for x in range(width):
            flat_pixels.append(pixel_data[y][x])
    img.putdata(flat_pixels)
    return img

print("1. Generating Angkor Wat tileset (assets/tiles_angkor.png)...")

# 16-color palette for Angkor Wat:
# 0: Transparent
# 1: Dark Slate outline / mortar (16, 20, 24)
# 2: Deep Sandstone shadow (48, 40, 32)
# 3: Mid Sandstone (120, 96, 64)
# 4: Warm Sandstone body (176, 144, 96)
# 5: Sunlit Sandstone highlight (232, 200, 144)
# 6: Moss / Foliage dark olive (32, 64, 24)
# 7: Moss / Jungle leaf bright (80, 160, 48)
# 8: Jungle leaf bright tip (152, 216, 72)
# 9: Loamy soil rich brown (96, 56, 32)
# 10: Soil terracotta highlight (152, 96, 56)
# 11: Ruby bright red (224, 32, 32)
# 12: Ruby deep wine (128, 16, 24)
# 13: Gold / Brass accent (240, 192, 40)
# 14: Pure Specular White glint (248, 248, 248)
# 15: Mystic Cyan Portal glow (48, 208, 240)

angkor_pal = [
    (0, 0, 0),        # 0: Transparent
    (20, 20, 24),     # 1: Deep mortar / crevice shadow
    (48, 44, 46),     # 2: Dark stone shadow
    (96, 90, 84),     # 3: Medium warm sandstone (floor base)
    (144, 136, 124),  # 4: Warm sandstone body
    (188, 178, 160),  # 5: Light sandstone face
    (232, 222, 204),  # 6: Sunlit stone rim highlight
    (56, 36, 24),     # 7: Dark loam / soil shadow
    (100, 64, 40),    # 8: Rich brown soil mid
    (144, 96, 60),    # 9: Warm loam crumb
    (44, 172, 80),    # 10: Radiant Emerald Green
    (228, 32, 40),    # 11: Ruby bright red
    (132, 16, 24),    # 12: Ruby deep wine
    (240, 192, 40),   # 13: Gold / Brass accent
    (252, 252, 252),  # 14: Pure Specular White glint
    (48, 204, 240)    # 15: Mystic Cyan Portal
]

# We will create an image 256 x 32 containing 16x16 metatiles
# Metatiles row 0 (x=0..255, y=0..15): 16 metatiles
# Metatiles row 1 (x=0..255, y=16..31): 16 metatiles (bedrock, pillar, relief, etc.)
angkor_pixels = [[0 for _ in range(256)] for _ in range(32)]

def draw_angkor_metatile(mx, my, grid):
    ox = mx * 16
    oy = my * 16
    for y in range(16):
        for x in range(16):
            angkor_pixels[oy + y][ox + x] = grid[y][x]

# 0: Angkor Floor (Smooth warm sandstone paving, seamless tiling)
m_floor = [[3 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        # Subtle 2-slab division
        if y == 7:
            m_floor[y][x] = 2
        elif y == 8:
            m_floor[y][x] = 4
        elif x == 7 and y < 7:
            m_floor[y][x] = 2
        elif x == 8 and y < 7:
            m_floor[y][x] = 4
        elif x == 15 and y >= 8:
            m_floor[y][x] = 2
        else:
            n = (x * 13 + y * 29) % 31
            if n == 0: m_floor[y][x] = 2
            elif n == 5: m_floor[y][x] = 4
            else: m_floor[y][x] = 3
draw_angkor_metatile(0, 0, m_floor)

# 1: Angkor Wall (Heavy ancient carved temple sandstone masonry)
m_wall = [[4 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        # 2 stone brick courses: Top (y:0..7), Bottom (y:8..15)
        is_mortar = (y == 7 or y == 15)
        if y < 7:
            if x == 7 or x == 15: is_mortar = True
        else:
            if x == 3 or x == 11: is_mortar = True
            
        if is_mortar:
            m_wall[y][x] = 1 # deep mortar
        elif (y == 0) or (y == 8):
            m_wall[y][x] = 6 if ((x % 8) in [1, 2]) else 5
        elif (y == 6) or (y == 14) or (x in [6, 14] and y < 7) or (x in [2, 10] and y >= 8):
            m_wall[y][x] = 2
        else:
            val = 4
            if ((x ^ y) % 5 == 0): val = 5
            elif ((x * 3 + y * 7) % 11 == 0): val = 3
            if (y in [3, 4] and x in [3, 4, 11, 12]): val = 3
            m_wall[y][x] = val
draw_angkor_metatile(1, 0, m_wall)

# 2: Angkor Dirt (Rich crumbly earth / soft diggable soil, no neon dots)
m_dirt = [[8 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        h = (x * 17 + y * 31 + (x ^ y) * 7) % 37
        if h < 8:
            m_dirt[y][x] = 7 # dark soil shadow
        elif h < 24:
            m_dirt[y][x] = 8 # rich loam body
        elif h < 33:
            m_dirt[y][x] = 9 # crumbly soil highlight
        else:
            m_dirt[y][x] = 4 # fine sandstone pebble
        if abs(y - ((x * 3 + 2) % 13)) == 0 and (x % 5 != 0):
            m_dirt[y][x] = 7
draw_angkor_metatile(2, 0, m_dirt)

# 3: Angkor Boulder (Chiseled spherical granite boulder)
m_boulder = [row[:] for row in m_floor]
for y in range(16):
    for x in range(16):
        dx = x - 7.5
        dy = y - 7.5
        dist2 = dx * dx + dy * dy
        if dist2 <= 49.0:
            nx = dx / 7.0
            ny = dy / 7.0
            nz = (max(0.0, 1.0 - nx*nx - ny*ny)) ** 0.5
            light = -0.55 * nx - 0.55 * ny + 0.6 * nz
            
            if light > 0.75: col = 6
            elif light > 0.45: col = 5
            elif light > 0.15: col = 4
            elif light > -0.2: col = 3
            else: col = 2
                
            if dist2 >= 42.0: col = 1
                
            if (x in [4, 5] and y in [4, 5]):
                col = 14 if (x == 4 and y == 4) else 6
                
            if abs(dy - (dx * 0.45 - 0.5)) < 0.65 and dx > -3 and dx < 4:
                col = 2
                
            m_boulder[y][x] = col
        elif dist2 <= 62.0:
            if (dx + dy) > 2.0:
                m_boulder[y][x] = 2
draw_angkor_metatile(3, 0, m_boulder)

def make_gem(base_col, dark_col, high_col):
    g = [row[:] for row in m_floor]
    for y in range(16):
        for x in range(16):
            dx = abs(x - 7.5)
            dy = abs(y - 7.5)
            manhattan = dx + dy
            if manhattan <= 6.5:
                g[y][x] = base_col
                if y < 7 and x < 7: g[y][x] = high_col
                elif y > 8 and x > 8: g[y][x] = dark_col
                elif y > 8 and x < 7: g[y][x] = dark_col
                if dx <= 2.2 and dy <= 2.2: g[y][x] = base_col
                if (x == 5 and y == 5) or (x == 6 and y == 5): g[y][x] = 14
                if manhattan >= 5.5: g[y][x] = 1
            elif manhattan <= 8.5:
                if (x >= 7 and y >= 7): g[y][x] = 2
    return g

# 4: Red Ruby
draw_angkor_metatile(4, 0, make_gem(11, 12, 11))
# 5: Purple Amethyst
draw_angkor_metatile(5, 0, make_gem(12, 1, 11))
# 6: Green Emerald
draw_angkor_metatile(6, 0, make_gem(10, 2, 10))

# 7: Golden Key (elaborate Cambodian temple brass key)
m_key = [row[:] for row in m_floor]
for y in range(16):
    for x in range(16):
        # Ring at top: center (7.5, 4.5)
        d2 = (x - 7.5)**2 + (y - 4.5)**2
        if d2 <= 16.0:
            if d2 > 4.0:
                m_key[y][x] = 13
                if x < 7 or y < 4: m_key[y][x] = 14 # highlight
                elif x > 8 or y > 5: m_key[y][x] = 2 # shadow
        # Stem: x in [7, 8], y in [7..13]
        if (x == 7 or x == 8) and (y >= 7 and y <= 13):
            m_key[y][x] = 13
            if x == 7: m_key[y][x] = 14
        # Teeth: x in [9..11], y in [10, 12]
        if (x >= 9 and x <= 11) and (y == 10 or y == 12):
            m_key[y][x] = 13
draw_angkor_metatile(7, 0, m_key)

# 8: Locked Door (Ancient Bronze Portcullis with carved stone archway)
m_door_locked = [[4 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        # Stone arch frame
        if x <= 2 or x >= 13 or y <= 2:
            m_door_locked[y][x] = 5 if (x == 0 or y == 0) else 3
            if x == 2 or x == 13 or y == 2: m_door_locked[y][x] = 1
        else:
            # Dark passage behind bars
            m_door_locked[y][x] = 1
            # Vertical iron bars
            if x in [5, 8, 10]:
                m_door_locked[y][x] = 13 # brass bar
                if y % 4 == 0: m_door_locked[y][x] = 14 # rivet
            # Horizontal reinforcing bar
            if y == 8: m_door_locked[y][x] = 13
            # Heavy padlock in center
            if x >= 7 and x <= 9 and y >= 7 and y <= 10:
                m_door_locked[y][x] = 13
                if y == 8 and x == 8: m_door_locked[y][x] = 1
draw_angkor_metatile(8, 0, m_door_locked)

# 9: Open Door (raised gate opening into shadowy ruin portal)
m_door_open = [[4 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        if x <= 2 or x >= 13 or y <= 2:
            m_door_open[y][x] = 5 if (x == 0 or y == 0) else 3
            if x == 2 or x == 13 or y == 2: m_door_open[y][x] = 1
        else:
            # Deep black portal with steps
            m_door_open[y][x] = 1
            if y >= 12: m_door_open[y][x] = 2 # steps
            if y >= 14: m_door_open[y][x] = 3
draw_angkor_metatile(9, 0, m_door_open)

# 10: Treasure Chest Closed (carved tropical ironwood, brass bands, lock)
m_chest_closed = [row[:] for row in m_floor]
for y in range(16):
    for x in range(16):
        if x >= 1 and x <= 14 and y >= 3 and y <= 14:
            # Wood planks
            m_chest_closed[y][x] = 9
            if y == 3 or y == 8: m_chest_closed[y][x] = 10 # wood highlight
            if y == 7 or y == 14: m_chest_closed[y][x] = 2 # shadow line
            # Brass bands
            if x in [2, 7, 8, 13]:
                m_chest_closed[y][x] = 13
                if y in [4, 9, 13]: m_chest_closed[y][x] = 14 # rivet
            # Gold lock latch
            if x in [7, 8] and y in [7, 8, 9]:
                m_chest_closed[y][x] = 13
                if y == 8: m_chest_closed[y][x] = 14
            # Outline
            if x == 1 or x == 14 or y == 3 or y == 14:
                m_chest_closed[y][x] = 1
draw_angkor_metatile(10, 0, m_chest_closed)

# 11: Treasure Chest Open (lid open, overflowing with glittering gold and gems)
m_chest_open = [row[:] for row in m_floor]
for y in range(16):
    for x in range(16):
        # Open lid tilted up (y: 1..5)
        if x >= 2 and x <= 13 and y >= 1 and y <= 4:
            m_chest_open[y][x] = 10
            if y == 1: m_chest_open[y][x] = 13 # brass rim
            if x in [3, 12]: m_chest_open[y][x] = 13
            if x == 2 or x == 13 or y == 1: m_chest_open[y][x] = 1
        # Overflowing treasure jewels (y: 5..8)
        if x >= 3 and x <= 12 and y >= 5 and y <= 8:
            m_chest_open[y][x] = 13 # gold coins
            if (x + y) % 3 == 0: m_chest_open[y][x] = 11 # ruby
            if (x * y) % 5 == 0: m_chest_open[y][x] = 14 # glint
        # Chest base (y: 9..14)
        if x >= 1 and x <= 14 and y >= 9 and y <= 14:
            m_chest_open[y][x] = 9
            if x in [2, 7, 8, 13]: m_chest_open[y][x] = 13
            if x == 1 or x == 14 or y == 14: m_chest_open[y][x] = 1
draw_angkor_metatile(11, 0, m_chest_open)

# 12: Ground Spikes (sharp triangular steel/iron spikes on stone plate)
m_spikes = [row[:] for row in m_floor]
for y in range(16):
    for x in range(16):
        # Stone base plate
        if y >= 12:
            m_spikes[y][x] = 4 if y == 12 else 2
        # 3 Spikes: centers at x=3, x=8, x=13
        for sx in [3, 8, 13]:
            dx = abs(x - sx)
            spike_h = 10 - dx * 3 # height
            if y >= 12 - spike_h and y < 12 and dx <= 2:
                if x < sx: m_spikes[y][x] = 14 # bright steel side
                elif x == sx: m_spikes[y][x] = 5 # tip / ridge
                else: m_spikes[y][x] = 1 # shadow steel side
draw_angkor_metatile(12, 0, m_spikes)

# 13: Pressure Plate (ancient stone slab with carved sun rune)
m_plate = [[3 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        # Recessed surrounding cavity
        if x == 0 or x == 15 or y == 0 or y == 15:
            m_plate[y][x] = 1 # deep shadow groove
        elif x == 1 or y == 1:
            m_plate[y][x] = 2
        else:
            # Raised beveled stone plate
            if x == 2 or y == 2: m_plate[y][x] = 5 # highlight
            elif x == 14 or y == 14: m_plate[y][x] = 2 # shadow
            else:
                m_plate[y][x] = 4
                # Carved geometric rune in center
                dx = abs(x - 8)
                dy = abs(y - 8)
                if dx + dy == 3 or (dx == 0 and dy <= 2) or (dy == 0 and dx <= 2):
                    m_plate[y][x] = 1
draw_angkor_metatile(13, 0, m_plate)

# 14: Retractable Barrier (heavy stone carved temple column)
m_barrier = [[4 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        # Fluted stone column
        if x == 0 or y == 0: m_barrier[y][x] = 5
        elif x == 15 or y == 15: m_barrier[y][x] = 1
        elif x in [3, 7, 11]: m_barrier[y][x] = 2 # vertical fluting groove
        elif x in [4, 8, 12]: m_barrier[y][x] = 5 # fluting ridge highlight
        else: m_barrier[y][x] = 4
        # Column capital & base bands
        if y in [2, 13]: m_barrier[y][x] = 13 # bronze reinforcing collar
        if y in [3, 14]: m_barrier[y][x] = 1
draw_angkor_metatile(14, 0, m_barrier)

# 15: Ancient Exit Gate (sacred Angkor portal with glowing blue celestial core)
m_portal = [[4 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        # Carved Naga stone arch frame
        if x <= 2 or x >= 13 or y <= 2:
            m_portal[y][x] = 5 if (x == 0 or y == 0) else 3
            if x == 2 or x == 13 or y == 2: m_portal[y][x] = 1
            # Carved eye rune on arch keystone
            if y <= 2 and x in [7, 8]: m_portal[y][x] = 13
        else:
            m_portal[y][x] = 1 # deep shadowy portal opening
            # Pulsating celestial portal swirl
            dx = x - 7.5
            dy = y - 8.5
            d2 = dx * dx + dy * dy
            if d2 <= 25:
                if d2 <= 4: m_portal[y][x] = 14 # white hot core
                elif d2 <= 12: m_portal[y][x] = 15 # bright cyan
                else: m_portal[y][x] = 2 # outer vortex
draw_angkor_metatile(15, 0, m_portal)

# Row 1: Bedrock / BG2 Parallax temple pillars and reliefs
# 16 & 17: Massive Sandstone Temple Pillar (Left & Right 16px metatiles)
m_pillar_l = [[3 for _ in range(16)] for _ in range(16)]
m_pillar_r = [[2 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        # Shaded column with rounded cylindrical gradient
        m_pillar_l[y][x] = 4 if x > 6 else 3
        if x < 2: m_pillar_l[y][x] = 2
        m_pillar_r[y][x] = 3 if x < 10 else 2
        if x > 13: m_pillar_r[y][x] = 1
        # Horizontal stone joints
        if y == 0 or y == 8:
            m_pillar_l[y][x] = 1
            m_pillar_r[y][x] = 1
draw_angkor_metatile(0, 1, m_pillar_l)
draw_angkor_metatile(1, 1, m_pillar_r)

# 18: Angkor Buddha Stone Relief Face (carved into bedrock)
m_face = [[3 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        m_face[y][x] = 2 # bedrock shadow
        # Carved serene face features
        if x >= 4 and x <= 11 and y >= 2 and y <= 13:
            m_face[y][x] = 3
            # Eyes
            if y == 5 and x in [5, 6, 9, 10]: m_face[y][x] = 1
            # Eyebrows
            if y == 4 and x in [5, 6, 7, 8, 9, 10]: m_face[y][x] = 4
            # Nose
            if (y >= 6 and y <= 9) and x in [7, 8]: m_face[y][x] = 4
            if y == 9 and x in [6, 9]: m_face[y][x] = 1
            # Lips
            if y == 11 and x in [6, 7, 8, 9]: m_face[y][x] = 4
            if y == 12 and x in [7, 8]: m_face[y][x] = 1
draw_angkor_metatile(2, 1, m_face)

# 19: Ancient Foundation Masonry Bedrock (repeating brickwork)
m_bedrock = [[2 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        # Brick joints
        row = y // 4
        if y % 4 == 0: m_bedrock[y][x] = 1
        elif (row % 2 == 0 and x == 8) or (row % 2 == 1 and (x == 0 or x == 15)):
            m_bedrock[y][x] = 1
        else:
            m_bedrock[y][x] = 3 if ((x + y) % 5 == 0) else 2
draw_angkor_metatile(3, 1, m_bedrock)

# Save Angkor image
img_angkor = create_indexed_image(256, 32, angkor_pal, angkor_pixels)
img_angkor.save("assets/tiles_angkor.png")
print("  -> Saved assets/tiles_angkor.png")


print("2. Generating Bavaria Catacombs tileset (assets/tiles_bavaria.png)...")

# 16-color palette for Bavaria (Gothic Dark Granite, Cast Iron, Blood Ruby):
bavaria_pal = [
    (0, 0, 0),        # 0: Transparent
    (16, 16, 20),     # 1: Deep mortar / crevice shadow
    (36, 40, 48),     # 2: Dark stone shadow
    (68, 72, 84),     # 3: Medium cold granite (floor base)
    (108, 116, 132),  # 4: Light granite body
    (152, 160, 176),  # 5: Granite face
    (208, 216, 228),  # 6: Rim highlight
    (48, 44, 40),     # 7: Catacomb dust/soil shadow
    (84, 76, 70),     # 8: Catacomb dry earth mid
    (124, 116, 108),  # 9: Catacomb earth light
    (44, 168, 76),    # 10: Emerald / poison green
    (224, 28, 36),    # 11: Blood ruby bright
    (128, 16, 24),    # 12: Blood ruby deep
    (236, 184, 36),   # 13: Gold relic
    (252, 252, 252),  # 14: Pure specular white
    (48, 204, 240)    # 15: Mystic cyan portal
]

bavaria_pixels = [[0 for _ in range(256)] for _ in range(32)]
def draw_bavaria_metatile(mx, my, grid):
    ox = mx * 16
    oy = my * 16
    for y in range(16):
        for x in range(16):
            bavaria_pixels[oy + y][ox + x] = grid[y][x]

# Bavaria uses the exact same metatile layout rendered with the gothic granite palette
for mx in range(16):
    for my in range(2):
        grid = [[angkor_pixels[my * 16 + y][mx * 16 + x] for x in range(16)] for y in range(16)]
        draw_bavaria_metatile(mx, my, grid)

# Save Bavaria image
img_bavaria = create_indexed_image(256, 32, bavaria_pal, bavaria_pixels)
img_bavaria.save("assets/tiles_bavaria.png")
print("  -> Saved assets/tiles_bavaria.png")


print("3. Generating Explorer sprite sheet (assets/sprites_explorer.png)...")

# Explorer 16-color OBJ Palette:
# 0: Transparent
# 1: Black outline (16, 16, 16)
# 2: Fedora dark brown / satchel leather (72, 36, 16)
# 3: Fedora tan / belt (152, 96, 48)
# 4: Fedora light / satchel highlight (216, 152, 80)
# 5: Skin deep shadow (144, 88, 64)
# 6: Skin tone (224, 160, 120)
# 7: Skin highlight (248, 208, 176)
# 8: Jacket brown / boots dark (48, 24, 12)
# 9: Explorer vest / khaki shirt shadow (112, 128, 136)
# 10: Khaki shirt bright (184, 200, 208)
# 11: Pants dark khaki (120, 104, 72)
# 12: Pants light khaki (176, 160, 120)
# 13: Brass belt buckle / button (240, 192, 40)
# 14: White specular / eye glint (248, 248, 248)
# 15: Hurt / squash red flash (224, 32, 32)

explorer_pal = [
    (0, 0, 0),        # 0: Transparent
    (16, 16, 16),     # 1: Outline
    (72, 36, 16),     # 2: Leather dark
    (152, 96, 48),    # 3: Fedora tan
    (216, 152, 80),   # 4: Fedora light
    (144, 88, 64),    # 5: Skin shadow
    (224, 160, 120),  # 6: Skin tone
    (248, 208, 176),  # 7: Skin highlight
    (48, 24, 12),     # 8: Jacket / boots
    (104, 120, 128),  # 9: Khaki shirt shadow
    (184, 200, 208),  # 10: Khaki shirt bright
    (120, 104, 72),   # 11: Pants shadow
    (176, 160, 120),  # 12: Pants bright
    (240, 192, 40),   # 13: Buckle gold
    (248, 248, 248),  # 14: Specular white
    (224, 32, 32)     # 15: Hurt red
]

# Sheet size: 256 x 16 (16 sprites of 16x16 pixels)
# 0: Idle Down
# 1..3: Walk Down 0, 1, 2
# 4: Idle Up
# 5..7: Walk Up 0, 1, 2
# 8: Idle Side
# 9..11: Walk Side 0, 1, 2
# 12: Push Stance
# 13: Push Strain
# 14: Squash / Hurt
# 15: Victory Pose

explorer_pixels = [[0 for _ in range(256)] for _ in range(16)]

def draw_explorer_sprite(idx, grid):
    ox = idx * 16
    for y in range(16):
        for x in range(16):
            explorer_pixels[y][ox + x] = grid[y][x]

def make_explorer_down(step_phase):
    # step_phase: 0 = idle/contact left, 1 = passing, 2 = contact right
    s = [[0 for _ in range(16)] for _ in range(16)]
    # Fedora Hat (y: 1..5)
    # Crown (y: 1..3, x: 5..10)
    for y in range(1, 4):
        for x in range(5, 11):
            s[y][x] = 3
            if y == 1 or x == 5: s[y][x] = 4
            if y == 3: s[y][x] = 2 # Hat band
    # Brim (y: 4, x: 3..12)
    for x in range(3, 13):
        s[4][x] = 3
        if x in [3, 12]: s[4][x] = 1 # outline
        elif x in [4, 5]: s[4][x] = 4
        else: s[4][x] = 2
    # Face & Satchel strap (y: 5..7, x: 5..10)
    for y in range(5, 8):
        for x in range(5, 11):
            s[y][x] = 6 # skin
            if y == 5 and x in [6, 9]: s[y][x] = 1 # eyes
            if y == 6 and x == 7: s[y][x] = 5 # nose
            if y == 7 and x in [7, 8]: s[y][x] = 5 # mouth
    # Torso & Jacket / Khaki Shirt (y: 8..11)
    for y in range(8, 12):
        for x in range(4, 12):
            s[y][x] = 10 # shirt
            if x in [4, 11]: s[y][x] = 8 # jacket sides
            # Satchel strap diagonal across chest
            if (x - 4) == (y - 8): s[y][x] = 2
            if y == 11 and x in [7, 8]: s[y][x] = 13 # belt buckle
    # Legs & Boots (y: 12..15)
    if step_phase == 0: # Idle / Left step forward
        for y in range(12, 14):
            s[y][5] = 12; s[y][6] = 11; s[y][9] = 12; s[y][10] = 11
        s[14][4] = 8; s[14][5] = 8; s[14][6] = 2 # Left boot
        s[15][4] = 1; s[15][5] = 8; s[15][6] = 1
        s[14][9] = 8; s[14][10] = 8 # Right boot
        s[15][9] = 1; s[15][10] = 8
    elif step_phase == 1: # Passing / mid-stride
        for y in range(12, 14):
            s[y][6] = 12; s[y][7] = 11; s[y][8] = 12; s[y][9] = 11
        s[14][6] = 8; s[14][7] = 8; s[15][6] = 1; s[15][7] = 8
        s[14][8] = 8; s[14][9] = 8; s[15][8] = 1; s[15][9] = 8
    elif step_phase == 2: # Right step forward
        for y in range(12, 14):
            s[y][5] = 12; s[y][6] = 11; s[y][9] = 12; s[y][10] = 11
        s[14][5] = 8; s[14][6] = 8; s[15][5] = 1; s[15][6] = 8
        s[14][9] = 8; s[14][10] = 8; s[14][11] = 2 # Right boot forward
        s[15][9] = 1; s[15][10] = 8; s[15][11] = 1

    # Black contour outline on external edges
    return s

draw_explorer_sprite(0, make_explorer_down(0))
draw_explorer_sprite(1, make_explorer_down(0))
draw_explorer_sprite(2, make_explorer_down(1))
draw_explorer_sprite(3, make_explorer_down(2))

def make_explorer_up(step_phase):
    s = [[0 for _ in range(16)] for _ in range(16)]
    # Fedora Hat Crown from behind (y: 1..3, x: 5..10)
    for y in range(1, 4):
        for x in range(5, 11):
            s[y][x] = 3
            if y == 1: s[y][x] = 4
            if y == 3: s[y][x] = 2
    # Brim (y: 4, x: 3..12)
    for x in range(3, 13):
        s[4][x] = 2
    # Hair / Neck (y: 5..7, x: 5..10)
    for y in range(5, 8):
        for x in range(5, 11):
            s[y][x] = 2 # dark hair
    # Back Jacket & Satchel Bag (y: 8..11, x: 4..11)
    for y in range(8, 12):
        for x in range(4, 12):
            s[y][x] = 8 # brown jacket
            if (x - 4) == (y - 8): s[y][x] = 3 # strap
            if x >= 9 and y >= 9: s[y][x] = 2 # satchel pouch on right hip
    # Legs (y: 12..15)
    if step_phase == 0:
        for y in range(12, 14): s[y][5] = 11; s[y][6] = 11; s[y][9] = 11; s[y][10] = 11
        s[14][5] = 8; s[14][6] = 8; s[15][5] = 1; s[15][6] = 1
        s[14][9] = 8; s[14][10] = 8; s[15][9] = 1; s[15][10] = 1
    elif step_phase == 1:
        for y in range(12, 14): s[y][6] = 11; s[y][7] = 11; s[y][8] = 11; s[y][9] = 11
        s[14][6] = 8; s[14][7] = 8; s[15][6] = 1; s[15][7] = 1
        s[14][8] = 8; s[14][9] = 8; s[15][8] = 1; s[15][9] = 1
    else:
        for y in range(12, 14): s[y][5] = 11; s[y][6] = 11; s[y][9] = 11; s[y][10] = 11
        s[14][5] = 8; s[14][6] = 8; s[15][5] = 1; s[15][6] = 1
        s[14][9] = 8; s[14][10] = 8; s[14][11] = 8; s[15][10] = 1; s[15][11] = 1
    return s

draw_explorer_sprite(4, make_explorer_up(0))
draw_explorer_sprite(5, make_explorer_up(0))
draw_explorer_sprite(6, make_explorer_up(1))
draw_explorer_sprite(7, make_explorer_up(2))

def make_explorer_side(step_phase):
    s = [[0 for _ in range(16)] for _ in range(16)]
    # Fedora in profile (y: 1..4, x: 4..12)
    for y in range(1, 4):
        for x in range(5, 11):
            s[y][x] = 3
            if y == 1 or x == 5: s[y][x] = 4
            if y == 3: s[y][x] = 2
    for x in range(3, 13):
        s[4][x] = 3
        if x in [3, 12]: s[4][x] = 1
        elif x >= 9: s[4][x] = 4 # brim sticking forward
    # Face profile (y: 5..7, x: 5..11)
    for y in range(5, 8):
        for x in range(6, 11):
            s[y][x] = 6
        if y == 5 and x == 9: s[y][x] = 1 # eye
        if y == 6 and x == 11: s[y][x] = 6 # nose tip
    # Jacket, arm swing & torso (y: 8..11, x: 5..11)
    for y in range(8, 12):
        for x in range(5, 11):
            s[y][x] = 8 # jacket
            if x == 7 and y == 11: s[y][x] = 13 # buckle
    # Legs swing
    if step_phase == 0:
        for y in range(12, 14): s[y][7] = 12; s[y][8] = 11
        s[14][6] = 8; s[14][7] = 8; s[15][6] = 1; s[15][7] = 1
    elif step_phase == 1: # Leg forward stride
        s[12][6] = 12; s[12][9] = 11
        s[13][5] = 12; s[13][10] = 11
        s[14][4] = 8; s[14][11] = 8
        s[15][4] = 1; s[15][11] = 1
    else: # Leg backward stride
        s[12][7] = 12; s[12][8] = 11
        s[13][8] = 12; s[13][9] = 11
        s[14][9] = 8; s[14][10] = 8
        s[15][9] = 1; s[15][10] = 1
    return s

draw_explorer_sprite(8, make_explorer_side(0))
draw_explorer_sprite(9, make_explorer_side(0))
draw_explorer_sprite(10, make_explorer_side(1))
draw_explorer_sprite(11, make_explorer_side(2))

# 12: Push Stance (leaning forward, hands outstretched)
s_push0 = make_explorer_side(1)
# Add outstretched hands at x=11..13, y=8..9
s_push0[8][11] = 6; s_push0[8][12] = 6; s_push0[8][13] = 7
s_push0[9][11] = 6; s_push0[9][12] = 6; s_push0[9][13] = 7
draw_explorer_sprite(12, s_push0)

# 13: Push Strain (straining hard against boulder)
s_push1 = make_explorer_side(1)
s_push1[8][12] = 7; s_push1[8][13] = 7; s_push1[8][14] = 14 # white knuckles
s_push1[9][12] = 7; s_push1[9][13] = 7; s_push1[9][14] = 14
draw_explorer_sprite(13, s_push1)

# 14: Squash / Hurt (pancake squashed flat with fedora on top)
s_squash = [[0 for _ in range(16)] for _ in range(16)]
# Fedora on top of squashed body (y: 8..10, x: 4..11)
for y in range(8, 11):
    for x in range(4, 12):
        s_squash[y][x] = 3 if y == 8 else 2
# Squashed pancake body (y: 11..15, x: 1..14)
for y in range(11, 16):
    for x in range(1, 15):
        s_squash[y][x] = 15 # red hurt flash
        if y >= 12 and y <= 14 and x >= 3 and x <= 12:
            s_squash[y][x] = 6 # skin
        if y == 15: s_squash[y][x] = 1
draw_explorer_sprite(14, s_squash)

# 15: Victory Pose (waving hat in air!)
s_vic = make_explorer_down(0)
# Raise arm with fedora at x=11..14, y=1..4
for y in range(1, 5):
    for x in range(11, 15):
        s_vic[y][x] = 3
draw_explorer_sprite(15, s_vic)

img_explorer = create_indexed_image(256, 16, explorer_pal, explorer_pixels)
img_explorer.save("assets/sprites_explorer.png")
print("  -> Saved assets/sprites_explorer.png")


print("4. Generating Enemies sprite sheet (assets/sprites_enemies.png)...")

# Enemies Palette (Cobra & Spider):
# 0: Transparent
# 1: Black outline (16, 16, 16)
# 2: Viper deep dorsal scales (20, 60, 20)
# 3: Viper mid green (40, 140, 40)
# 4: Viper vibrant emerald (80, 210, 60)
# 5: Viper cream yellow belly (240, 220, 90)
# 6: Viper ruby eye / forked tongue (230, 30, 30)
# 7: Spider deep brown body (48, 28, 16)
# 8: Spider mid chitin (104, 60, 32)
# 9: Spider striped leg orange (176, 104, 48)
# 10: Spider glowing crimson eye (240, 40, 40)
# 11: Spider fang bone white (248, 248, 248)
# 12..15: Unused / pad
enemy_pal = [
    (0, 0, 0),        # 0: Transparent
    (16, 16, 16),     # 1: Outline
    (20, 60, 20),     # 2: Dark green
    (40, 140, 40),    # 3: Mid green
    (80, 210, 60),    # 4: Bright emerald
    (240, 220, 90),   # 5: Yellow belly
    (230, 30, 30),    # 6: Ruby red
    (48, 28, 16),     # 7: Spider dark
    (104, 60, 32),    # 8: Spider mid
    (176, 104, 48),   # 9: Spider leg orange
    (240, 40, 40),    # 10: Red eye
    (248, 248, 248),  # 11: White fang
    (0, 0, 0), (0, 0, 0), (0, 0, 0), (0, 0, 0)
]

enemy_pixels = [[0 for _ in range(128)] for _ in range(16)]
def draw_enemy_sprite(idx, grid):
    ox = idx * 16
    for y in range(16):
        for x in range(16):
            enemy_pixels[y][ox + x] = grid[y][x]

# 0..3: Cambodian Cobra Slither & Strike (smooth S-curve oscillation)
for frame in range(4):
    s = [[0 for _ in range(16)] for _ in range(16)]
    # Coiled body along bottom (y: 8..15)
    for x in range(1, 14):
        # S-curve oscillation based on frame
        wave = int(2.0 * ((x + frame * 3) % 6 < 3))
        by = 12 + wave - 1
        for dy in range(3):
            y = by + dy
            if y < 16:
                s[y][x] = 3
                if dy == 0: s[y][x] = 4 # dorsal highlight
                if dy == 2: s[y][x] = 5 # yellow belly
                if x == 1 or y == 15: s[y][x] = 1 # outline
    # Cobra Hood & Head (raised on right side, x: 10..15, y: 5..10)
    for y in range(5, 11):
        for x in range(11, 15):
            s[y][x] = 3
            if x == 14: s[y][x] = 4
            if x == 11 and y in [7, 8]: s[y][x] = 2 # hood curve
    # Eye & Tongue
    s[6][13] = 6 # red eye
    s[6][14] = 1 # pupil
    if frame in [1, 3]:
        # Flicking forked tongue!
        s[8][15] = 6
        if frame == 3: s[7][15] = 6
    draw_enemy_sprite(frame, s)

# 4..7: Bavarian Chitin Spider Crawl (8 legs scuttling vertically/horizontally)
for frame in range(4):
    s = [[0 for _ in range(16)] for _ in range(16)]
    # Abdomen & Cephalothorax (center at x: 8, y: 8)
    for y in range(5, 12):
        for x in range(5, 12):
            d2 = (x - 8)**2 + (y - 8)**2
            if d2 <= 14:
                s[y][x] = 8
                if d2 <= 4: s[y][x] = 9
                if d2 >= 10: s[y][x] = 7
    # Multi-cluster red glowing eyes
    s[7][6] = 10; s[7][7] = 10; s[8][6] = 10
    # White fangs
    s[6][5] = 11; s[9][5] = 11
    # 8 Articulated Legs (twitching per frame)
    leg_offsets = [
        [(2, 3), (2, 6), (2, 10), (2, 13), (13, 3), (13, 6), (13, 10), (13, 13)],
        [(3, 2), (2, 7), (3, 10), (2, 14), (12, 2), (13, 7), (12, 10), (13, 14)],
        [(2, 2), (3, 6), (2, 11), (3, 13), (13, 2), (12, 6), (13, 11), (12, 13)],
        [(3, 3), (2, 5), (3, 9), (2, 13), (12, 3), (13, 5), (12, 9), (13, 13)]
    ][frame]
    for lx, ly in leg_offsets:
        s[ly][lx] = 9
        # Joint to body
        s[(ly + 8)//2][(lx + 8)//2] = 8
    draw_enemy_sprite(4 + frame, s)

img_enemies = create_indexed_image(128, 16, enemy_pal, enemy_pixels)
img_enemies.save("assets/sprites_enemies.png")
print("  -> Saved assets/sprites_enemies.png")


print("5. Generating Objects & FX sprite sheet (assets/sprites_objects.png)...")

# Objects Palette (Rolling Boulders, Gems, Sparkles, Dust, Goo):
# 0: Transparent
# 1: Dark Slate outline (24, 24, 32)
# 2: Boulder shadow (56, 48, 40)
# 3: Boulder mid stone (128, 108, 88)
# 4: Boulder light highlight (208, 184, 152)
# 5: Ruby bright red (224, 32, 32)
# 6: Amethyst purple (160, 48, 208)
# 7: Emerald green (48, 208, 64)
# 8: Pure Specular White glint (248, 248, 248)
# 9: Sparkle gold / brass (248, 208, 40)
# 10: Dust cloud dark tan (120, 88, 56)
# 11: Dust cloud light tan (184, 152, 112)
# 12: Green snake squash goo (48, 192, 48)
# 13: Deep goo puddle (24, 96, 24)
# 14..15: Unused
obj_fx_pal = [
    (0, 0, 0),        # 0: Transparent
    (20, 20, 24),     # 1: Outline
    (48, 44, 46),     # 2: Shadow
    (96, 90, 84),     # 3: Mid stone
    (188, 178, 160),  # 4: Highlight
    (228, 32, 40),    # 5: Ruby bright
    (132, 16, 24),    # 6: Ruby deep
    (44, 172, 80),    # 7: Emerald
    (252, 252, 252),  # 8: White glint
    (240, 192, 40),   # 9: Gold sparkle
    (100, 64, 40),    # 10: Dust dark
    (144, 96, 60),    # 11: Dust light
    (48, 192, 48),    # 12: Green goo
    (24, 96, 24),     # 13: Dark goo
    (0, 0, 0), (0, 0, 0)
]

obj_fx_pixels = [[0 for _ in range(256)] for _ in range(16)]
def draw_obj_sprite(idx, grid):
    ox = idx * 16
    for y in range(16):
        for x in range(16):
            obj_fx_pixels[y][ox + x] = grid[y][x]

# 0..3: Rolling Boulder 4 Rotational Frames (0 deg, 90 deg, 180 deg, 270 deg)
import math
for r_frame in range(4):
    angle = r_frame * (math.pi / 2.0)
    s = [[0 for _ in range(16)] for _ in range(16)]
    for y in range(16):
        for x in range(16):
            dx = x - 7.5
            dy = y - 7.5
            d2 = dx * dx + dy * dy
            if d2 <= 49.0:
                s[y][x] = 3
                # Spherical light from top-left
                if dx + dy > 2: s[y][x] = 2
                if dx + dy < -2: s[y][x] = 4
                if d2 >= 42.0: s[y][x] = 1
                # Rotational surface fissure: rotate coordinates by angle
                rx = dx * math.cos(angle) - dy * math.sin(angle)
                ry = dx * math.sin(angle) + dy * math.cos(angle)
                if abs(ry - 0.4 * rx) < 0.75 and d2 < 36:
                    s[y][x] = 2 # subtle crack in shadow tone
    draw_obj_sprite(r_frame, s)

# 4: Falling Ruby Sprite
s_ruby = [[0 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        m = abs(x - 7.5) + abs(y - 7.5)
        if m <= 6.5:
            s_ruby[y][x] = 5
            if y < 7 and x < 7: s_ruby[y][x] = 4
            elif y > 8 and x > 8: s_ruby[y][x] = 6
            if (x == 5 and y == 5) or (x == 6 and y == 5): s_ruby[y][x] = 8
            if m >= 5.5: s_ruby[y][x] = 1
draw_obj_sprite(4, s_ruby)

# 5: Falling Amethyst Sprite
s_amethyst = [[0 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        m = abs(x - 7.5) + abs(y - 7.5)
        if m <= 6.5:
            s_amethyst[y][x] = 6
            if y < 7 and x < 7: s_amethyst[y][x] = 5
            elif y > 8 and x > 8: s_amethyst[y][x] = 1
            if (x == 5 and y == 5) or (x == 6 and y == 5): s_amethyst[y][x] = 8
            if m >= 5.5: s_amethyst[y][x] = 1
draw_obj_sprite(5, s_amethyst)

# 6: Falling Emerald Sprite
s_emerald = [[0 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        m = abs(x - 7.5) + abs(y - 7.5)
        if m <= 6.5:
            s_emerald[y][x] = 7
            if y < 7 and x < 7: s_emerald[y][x] = 8
            elif y > 8 and x > 8: s_emerald[y][x] = 2
            if (x == 5 and y == 5) or (x == 6 and y == 5): s_emerald[y][x] = 8
            if m >= 5.5: s_emerald[y][x] = 1
draw_obj_sprite(6, s_emerald)

# 7..9: Sparkle Star (Small, Cross, Large Glint)
for sp_frame in range(3):
    s = [[0 for _ in range(16)] for _ in range(16)]
    span = [2, 4, 6][sp_frame]
    for y in range(16):
        for x in range(16):
            dx = abs(x - 8)
            dy = abs(y - 8)
            if (dx == 0 and dy <= span) or (dy == 0 and dx <= span):
                s[y][x] = 8 # white cross
            if dx == dy and dx <= (span // 2):
                s[y][x] = 9 # gold diagonal
            if dx <= 1 and dy <= 1:
                s[y][x] = 8 # core
    draw_obj_sprite(7 + sp_frame, s)

# 10..12: Dig Dust / Leaf Debris Puffs (Small, Bursting with leaf particles, Dissipating)
for p_frame in range(3):
    s = [[0 for _ in range(16)] for _ in range(16)]
    r_max = [4, 6, 7][p_frame]
    for y in range(16):
        for x in range(16):
            d2 = (x - 8)**2 + (y - 8)**2
            if d2 <= r_max**2:
                s[y][x] = 10 if d2 > (r_max - 2)**2 else 11
                # Green leaf specks in excavation debris!
                if (x * 3 + y * 7) % 7 == 0:
                    s[y][x] = 7 # green leaf speck
    draw_obj_sprite(10 + p_frame, s)

# 13: Crushed Enemy Goo Splatter Decal (green goo residue)
s_goo = [[0 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        # Flattened puddle
        dx = abs(x - 7.5)
        dy = abs(y - 12)
        if dx * dx * 0.5 + dy * dy * 2 <= 14:
            s_goo[y][x] = 12
            if dy == 0 and dx <= 3: s_goo[y][x] = 13
draw_obj_sprite(13, s_goo)

# 14: Ground Oval Drop-Shadow (soft ambient occlusion under entities)
s_shadow = [[0 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        dx = x - 7.5
        dy = y - 11.5
        if (dx * dx) / 25.0 + (dy * dy) / 6.0 <= 1.0:
            s_shadow[y][x] = 2
            if (dx * dx) / 16.0 + (dy * dy) / 3.5 <= 1.0:
                s_shadow[y][x] = 1
draw_obj_sprite(14, s_shadow)

# 15: Landing Impact Dust Shockwave (expanding puff when boulders hit ground)
s_impact = [[0 for _ in range(16)] for _ in range(16)]
for y in range(16):
    for x in range(16):
        dx = abs(x - 7.5)
        dy = abs(y - 12)
        d = (dx * dx) / 36.0 + (dy * dy) / 8.0
        if d <= 1.0 and d >= 0.35:
            s_impact[y][x] = 11 if ((x + y) % 2 == 0) else 10
draw_obj_sprite(15, s_impact)

img_objects = create_indexed_image(256, 16, obj_fx_pal, obj_fx_pixels)
img_objects.save("assets/sprites_objects.png")
print("  -> Saved assets/sprites_objects.png")


print("6. Generating UI & Title Banner sheet (assets/ui_title.png)...")

# UI Palette (Stone HUD, Golden Title Logo, Heart/Gem Icons):
ui_pal = [
    (0, 0, 0),        # 0: Transparent
    (20, 20, 28),     # 1: Dark Slate border
    (48, 52, 64),     # 2: Deep Dark fill
    (112, 120, 136),  # 3: Bevel Light
    (248, 248, 248),  # 4: Crisp White text
    (248, 208, 40),   # 5: Golden Yellow
    (224, 32, 32),    # 6: Ruby Red
    (248, 144, 144),  # 7: Heart highlight
    (48, 208, 240),   # 8: Gem Cyan
    (184, 136, 48),   # 9: Key Bronze
    (80, 80, 96),     # 10: Gray font shadow
    (176, 144, 96),   # 11: Warm sandstone
    (144, 16, 24),    # 12: Dark ruby
    (248, 232, 128),  # 13: Pale gold rim
    (0, 0, 0), (0, 0, 0)
]

# We will generate HUD tiles (Diamond, Hearts, Key, Star, Box borders) in a 128x32 image
ui_pixels = [[0 for _ in range(128)] for _ in range(32)]
def draw_ui_tile(tx, ty, grid):
    ox = tx * 8
    oy = ty * 8
    for y in range(8):
        for x in range(8):
            ui_pixels[oy + y][ox + x] = grid[y][x]

# 8x8 Diamond Icon
d_icon = [
    [2,2,2,8,8,2,2,2],
    [2,2,8,4,4,8,2,2],
    [2,8,4,4,4,4,8,2],
    [8,4,4,4,4,4,4,8],
    [8,8,4,4,4,4,8,8],
    [2,8,8,4,4,8,8,2],
    [2,2,8,8,8,8,2,2],
    [2,2,2,8,8,2,2,2]
]
draw_ui_tile(0, 0, d_icon)

# 8x8 Heart Full
h_full = [
    [2,6,6,2,2,6,6,2],
    [6,7,7,6,6,7,7,6],
    [6,7,7,7,7,7,7,6],
    [6,7,7,7,7,7,7,6],
    [2,6,7,7,7,7,6,2],
    [2,2,6,7,7,6,2,2],
    [2,2,2,6,6,2,2,2],
    [2,2,2,2,2,2,2,2]
]
draw_ui_tile(1, 0, h_full)

# 8x8 Heart Empty
h_empty = [
    [2,1,1,2,2,1,1,2],
    [1,2,2,1,1,2,2,1],
    [1,2,2,2,2,2,2,1],
    [1,2,2,2,2,2,2,1],
    [2,1,2,2,2,2,1,2],
    [2,2,1,2,2,1,2,2],
    [2,2,2,1,1,2,2,2],
    [2,2,2,2,2,2,2,2]
]
draw_ui_tile(2, 0, h_empty)

# 8x8 Golden Key Icon
k_icon = [
    [2,2,2,5,5,2,2,2],
    [2,2,5,13,13,5,2,2],
    [2,2,2,5,5,2,2,2],
    [2,2,2,5,2,2,2,2],
    [2,2,2,5,5,2,2,2],
    [2,2,2,5,2,2,2,2],
    [2,2,2,5,5,2,2,2],
    [2,2,2,2,2,2,2,2]
]
draw_ui_tile(3, 0, k_icon)

# 8x8 Star Icon
star_icon = [
    [2,2,2,5,5,2,2,2],
    [2,2,2,5,5,2,2,2],
    [5,5,5,5,5,5,5,5],
    [2,5,5,5,5,5,5,2],
    [2,2,5,5,5,5,2,2],
    [2,5,5,2,2,5,5,2],
    [5,5,2,2,2,2,5,5],
    [2,2,2,2,2,2,2,2]
]
draw_ui_tile(4, 0, star_icon)

# Box Frame tiles (TL, TOP, TR, LEFT, RIGHT, BL, BOTTOM, BR, FILL)
b_tl = [[3 if (x == 0 or y == 0) else (1 if (x == 1 or y == 1) else 2) for x in range(8)] for y in range(8)]
b_top = [[3 if y == 0 else (1 if y == 1 else 2) for x in range(8)] for y in range(8)]
b_tr = [[1 if (x == 7 or y == 0) else (3 if (x == 6 or y == 1) else 2) for x in range(8)] for y in range(8)]
b_left = [[3 if x == 0 else (1 if x == 1 else 2) for x in range(8)] for y in range(8)]
b_right = [[1 if x == 7 else (3 if x == 6 else 2) for x in range(8)] for y in range(8)]
b_bl = [[1 if (x == 0 or y == 7) else (3 if (x == 1 or y == 6) else 2) for x in range(8)] for y in range(8)]
b_bottom = [[1 if y == 7 else (3 if y == 6 else 2) for x in range(8)] for y in range(8)]
b_br = [[1 if (x == 7 or y == 7) else (3 if (x == 6 or y == 6) else 2) for x in range(8)] for y in range(8)]
b_fill = [[2 for _ in range(8)] for _ in range(8)]

draw_ui_tile(5, 0, b_tl)
draw_ui_tile(6, 0, b_top)
draw_ui_tile(7, 0, b_tr)
draw_ui_tile(8, 0, b_left)
draw_ui_tile(9, 0, b_right)
draw_ui_tile(10, 0, b_bl)
draw_ui_tile(11, 0, b_bottom)
draw_ui_tile(12, 0, b_br)
draw_ui_tile(13, 0, b_fill)

# Expand ui_pixels to 160 x 48 to accommodate the 160x32 Title Banner in rows 2..5 (y: 16..47)
ui_pixels_expanded = [[0 for _ in range(160)] for _ in range(48)]
for y in range(32):
    for x in range(128):
        ui_pixels_expanded[y][x] = ui_pixels[y][x]

# Draw 160x32 Embossed Title Banner into y: 16..47, x: 0..159
# 1. Carved stone plaque base
for y in range(16, 48):
    for x in range(160):
        c = 2 # dark slate fill
        # Beveled stone rim
        if x == 0 or y == 16: c = 3
        elif x == 159 or y == 47: c = 1
        elif x == 1 or y == 17: c = 11 # sandstone trim
        elif x == 158 or y == 46: c = 1
        # Corner gold studs
        if (x in [3, 4] or x in [155, 156]) and (y in [19, 20] or y in [43, 44]):
            c = 5
        ui_pixels_expanded[y][x] = c

# 2. Embossed letters "DIAMOND" (y: 20..29) and "RUSH" (y: 31..42)
# Letter glyph definitions for 7x9 "DIAMOND"
logo_glyphs_diamond = {
    'D': ["111110", "110011", "110011", "110011", "110011", "110011", "110011", "111110"],
    'I': ["11111", "00100", "00100", "00100", "00100", "00100", "00100", "11111"],
    'A': ["001100", "011110", "110011", "110011", "111111", "110011", "110011", "110011"],
    'M': ["1100011", "1110111", "1111111", "1101011", "1100011", "1100011", "1100011", "1100011"],
    'O': ["011110", "110011", "110011", "110011", "110011", "110011", "110011", "011110"],
    'N': ["1100011", "1110011", "1111011", "1101111", "1100111", "1100011", "1100011", "1100011"]
}

def render_logo_text(text, glyphs, start_x, start_y, col_fill, col_hi, col_sh, col_out):
    cur_x = start_x
    for ch in text:
        grid = glyphs[ch]
        gh = len(grid)
        gw = len(grid[0])
        for r in range(gh):
            for c in range(gw):
                if grid[r][c] == '1':
                    # Drop shadow & outline
                    for ox, oy in [(-1, 0), (1, 0), (0, -1), (0, 1), (1, 1), (2, 2)]:
                        px = cur_x + c + ox
                        py = start_y + r + oy
                        if ui_pixels_expanded[py][px] != col_fill and ui_pixels_expanded[py][px] != col_hi:
                            ui_pixels_expanded[py][px] = col_sh if (ox > 0 and oy > 0) else col_out
        for r in range(gh):
            for c in range(gw):
                if grid[r][c] == '1':
                    px = cur_x + c
                    py = start_y + r
                    ui_pixels_expanded[py][px] = col_hi if (r < 2 or c < 2) else col_fill
        cur_x += gw + 2

# Draw "DIAMOND" in gold bevel (start_x = 48, start_y = 20)
render_logo_text("DIAMOND", logo_glyphs_diamond, 48, 20, 5, 13, 9, 1)

# Large bold "RUSH" glyphs (height 10)
logo_glyphs_rush = {
    'R': ["1111100", "1100110", "1100110", "1111100", "1111000", "1101100", "1100110", "1100111", "1100011"],
    'U': ["1100011", "1100011", "1100011", "1100011", "1100011", "1100011", "1100011", "0111110", "0011100"],
    'S': ["0111110", "1100011", "1100000", "0111100", "0001110", "0000011", "1100011", "0111110", "0011100"],
    'H': ["1100011", "1100011", "1100011", "1111111", "1111111", "1100011", "1100011", "1100011", "1100011"]
}
# Draw "RUSH" in vibrant crimson & gold (start_x = 58, start_y = 31)
render_logo_text("RUSH", logo_glyphs_rush, 58, 31, 6, 7, 12, 1)

# Central sparkling faceted ruby gem in logo at x=20..32, y=24..36
for y in range(24, 38):
    for x in range(20, 34):
        dx = abs(x - 26.5)
        dy = abs(y - 30.5)
        if dx + dy <= 5.5:
            c = 6 # ruby red
            if dx <= 1 and dy <= 1: c = 4 # white glint
            elif x < 26 and y < 30: c = 7 # highlight
            elif x > 27 and y > 31: c = 12 # dark ruby
            ui_pixels_expanded[y][x] = c
        elif dx + dy <= 6.5:
            ui_pixels_expanded[y][x] = 1 # outline

# Mirror diamond on right at x=126..140
for y in range(24, 38):
    for x in range(126, 140):
        dx = abs(x - 132.5)
        dy = abs(y - 30.5)
        if dx + dy <= 5.5:
            c = 6
            if dx <= 1 and dy <= 1: c = 4
            elif x < 132 and y < 30: c = 7
            elif x > 133 and y > 31: c = 12
            ui_pixels_expanded[y][x] = c
        elif dx + dy <= 6.5:
            ui_pixels_expanded[y][x] = 1

img_ui = create_indexed_image(160, 48, ui_pal, ui_pixels_expanded)
img_ui.save("assets/ui_title.png")
print("  -> Saved assets/ui_title.png")

print("All source image sheets successfully created in assets/!")

