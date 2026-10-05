# 💎 DIAMOND RUSH: Ancient Expeditions 🏛️🎮

A complete, hardware-accelerated **Action-Puzzle Adventure Game** built from scratch in C and ARM assembly for the **Game Boy Advance (ARM7TDMI)**, inspired by Gameloft's legendary mobile classic *Diamond Rush*.

---

## 🕹️ Game Overview

* **Platform:** Game Boy Advance (ARM7TDMI @ 16.78 MHz)
* **ROM File:** `diamond_rush.gba` (256 KB)
* **Framerate:** 60 FPS (Hardware VBlank synchronized)
* **Resolution:** 240 × 160 pixels
* **Display Engine:** 3-layer GBA Mode 0 architecture with Gameloft-grade visual fidelity:
  * **BG0:** Topmost HUD overlay (Carved stone plaque, Diamond Quota Counter, Beveled Hearts, Golden Key, Score, Level), 3D embossed Title Logo, dialogue boxes, and pause menus.
  * **BG1:** Dynamic interactive puzzle layer ($64 \times 32$ SBB tilemap) featuring multi-theme tilesets:
    * **Angkor Wat Ruins (Levels 1–3):** Weathered sandstone flagstones, bas-relief carvings, rich loamy soil with jungle vines, faceted sparkling rubies, amethysts, emeralds, bronze barred gates, and glowing exit portal.
    * **Catacombs of Bavaria (Level 4):** Dark gothic granite masonry, iron dungeon portcullis, aged cobwebs, and cold dungeon flagstones.
  * **BG2:** Subterranean architectural parallax bedrock with ancient carved columns and Buddha reliefs scrolling at half-speed for deep ruin depth.
  * **Hardware Effects:** Real-time VBlank palette shimmer (sparkling gem facets, pulsating mystic portal runes), screen shake, and GBA hardware alpha blending (`REG_BLDCNT`).
  * **High-Fidelity Sprites:** 16×16 Explorer with smooth 3-frame walk cycles (down, up, side), pushing stance & strain, squash pancake hurt pose, victory cheer; 4-frame animated Cambodian Cobras & Bavarian Spiders; 4-frame rotational rolling boulders; excavation debris, sparkle glints, and crushed green goo decals.
* **Asset Pipeline:** Automated Python compiler (`tools/build_assets.py`) compiling 16-color PNG sprite and tile sheets into hardware 4bpp planar tables and BGR555 palettes.
* **SRAM Persistence:** Battery backup save system (`0x0E000000`) persisting Campaign Progression (Levels 1–4), Career High Score, and total gems collected across expeditions.

---

## 🎮 Controller Layout

| Input | In-Game Action | Menu / Title Action |
| :--- | :--- | :--- |
| **D-Pad** | Smooth 4-directional stepped movement (Down, Up, Left, Right) | Navigate menu options / Cycle unlocked levels ($1 \dots \text{max\_unlocked}$) |
| **A Button** | Action / Confirm | Start Expedition / Confirm selection |
| **B Button** | Cancel / Unpause | Return to Title Screen from Game Over / Pause |
| **Start Button** | Pause Game (Resume / Restart Level / Quit) | Start Expedition from Title Screen |

---

## 💎 Core Mechanics & Physics (Gameloft Diamond Rush Style)

1. **Digging Soil & Foliage:**
   * Walking into soft brown soil or jungle vines instantly clears the tile with a rustling sound effect, leaving walkable empty space.
2. **Boulder Gravity & Crushing:**
   * Boulders (`META_BOULDER`) are heavy round stones subject to gravity.
   * If the tile below a boulder is cleared, the boulder falls.
   * Falling boulders will **squash enemies (Cobras and Spiders)** beneath them into green goo for bonus points (+300 pts)!
   * Falling boulders will also **crush the Explorer** if you linger underneath!
3. **Boulder Rolling (Sliding Off Curved Surfaces):**
   * If a boulder rests atop another boulder, diamond, or rounded stone, and the side and diagonal beneath are empty, the boulder rolls off to the side and falls down.
4. **Horizontal Pushing:**
   * The explorer can push boulders horizontally (Left or Right) if the space behind the boulder is empty or is a pressure plate.
   * Bracing push animation and low stone grinding sound effect accompany the push.
5. **Gems & Treasure Scoring:**
   * **Red Ruby:** +100 Points (counts toward stage quota)
   * **Purple Amethyst:** +250 Points
   * **Green Emerald:** +500 Points (rare secret stash)
   * **Treasure Chest:** Bursts open with 3 bonus diamonds and +500 points!
6. **Ancient Keys & Locked Doors:**
   * Collect the Golden Key to open the heavy stone portcullis blocking ancient chambers.
7. **Pressure Plates & Retractable Barriers:**
   * Pushing a boulder onto a stone pressure plate (or standing on it) depresses the plate with a mechanical click, lowering impassable stone barriers.
8. **Exit Portal & Quota:**
   * Each ruin requires a specific Diamond Quota to activate the ancient exit gate. Once satisfied, stepping into the gate triggers the stage victory fanfare!

---

## 🗺️ Expedition Levels

```
[Level 1: Angkor Entrance] ---> [Level 2: Boulder Chasm]
                                        |
[Level 4: Catacombs of Bavaria] <--- [Level 3: Serpent Sanctuary]
```

1. **Level 1 (Angkor Entrance):**
   * Introduction to soil digging, collecting red rubies, dropping boulders onto patrolling cobras, finding the golden key, and reaching the exit portal.
   * Quota: 8 Diamonds.
2. **Level 2 (Boulder Chasm):**
   * Advanced boulder stacking and roll-off puzzles, clearing chasm paths, collecting purple amethysts, and navigating multiple snake corridors.
   * Quota: 12 Diamonds.
3. **Level 3 (Serpent Sanctuary):**
   * Pressure plate mechanics (pushing boulders onto triggers to permanently lower stone barriers), vertical spider gauntlets, floor spikes, and secret green emeralds.
   * Quota: 14 Diamonds.
4. **Level 4 (Catacombs of Bavaria):**
   * The ultimate test of an adventurer: multi-tiered cascading boulder avalanches, multiple pressure switches, tight corridors where wrong moves can trap you, and the grand exit gate.
   * Quota: 16 Diamonds.

---

## 🛠️ Building and Running

### Prerequisites
Inside Ubuntu PRoot / Termux:
```bash
apt-get install gcc-arm-none-eabi binutils-arm-none-eabi make python3
```

### Compiling
```bash
cd /sdcard/Download/termux/gba-metroidvania/diamond_rush
make clean && make
```

The resulting file **`diamond_rush.gba`** is 100% hardware-compliant (checksum patched) and ready to run in any GBA emulator (mGBA, Pizza Boy GBA, RetroArch, My Boy!).
