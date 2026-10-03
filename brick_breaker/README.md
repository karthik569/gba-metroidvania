# 🧱 GBA Brick Breaker (Arkanoid Edition) 🎮

An action-packed, hardware-accelerated **Arkanoid-style Brick Breaker** built for the **Game Boy Advance (ARM7TDMI)**.

---

## 🕹️ Game Overview

* **Platform:** Game Boy Advance
* **ROM File:** `brick_breaker.gba` (256 KB)
* **Framerate:** 60 FPS (hardware VBlank locked)
* **Resolution:** 240 × 160 pixels
* **Audio:** Direct PSG hardware chiptune sound engine
* **SRAM:** High Score battery backup persistence (`0x0E000000`)

---

## 🎮 Controller Layout

| Input | Action | Behavior |
| :--- | :--- | :--- |
| **D-Pad Left / Right** | Move Paddle | Smooth, subpixel precision control |
| **A / B Button** | Launch / Fire | Launches ball from paddle / Fires Laser Cannons |
| **Start Button** | Pause / Unpause | Toggles in-game pause state |

---

## ⚡ Power-Up Capsules

Destroying colored and armored bricks has a chance to drop special capsules:

| Capsule | Name | Effect |
| :---: | :--- | :--- |
| **[W]** | **Wide Paddle** | Expands paddle width from 32px to 48px for easier saves. |
| **[L]** | **Laser Blaster** | Mounts twin laser cannons on paddle tips (press A/B to blast bricks). |
| **[M]** | **Multi-Ball** | Splits the active ball into 3 independently bouncing balls! |
| **[C]** | **Catch** | Sticky magnetic paddle that catches returning balls for aimed launch. |
| **[S]** | **Slow Ball** | Temporarily slows down high-velocity balls for tight control. |
| **[P]** | **Extra Life** | Awards +1 reserve paddle and 1,000 bonus points. |

---

## 🧱 Brick Varieties

* **Red Brick:** 100 points (1 hit)
* **Blue Brick:** 120 points (1 hit)
* **Green Brick:** 150 points (1 hit)
* **Yellow Brick:** 180 points (1 hit)
* **Magenta Brick:** 200 points (1 hit)
* **Cyan Brick:** 250 points (1 hit)
* **Silver Armored Brick:** 500 points (2 hits, cracks on first impact)
* **Gold Barrier Brick:** Indestructible obstacle block (deflects balls & lasers)

---

## 🗺️ Progressive Stages

1. **Stage 1 (Initiation Grid):** Classic alternating colored rows.
2. **Stage 2 (Space Invader):** Retro 8-bit alien brick formation.
3. **Stage 3 (Checkerboard Vault):** Alternating silver armor and high-value gems.
4. **Stage 4 (Diamond Fortress):** Diamond armor shell shielding high-value core bricks.
5. **Stage 5 (Golden Gauntlet):** Indestructible golden pillars channeling high-speed rebounds.
6. **Victory Screen:** Upon clearing Stage 5!

---

## 🛠️ Building

```bash
cd /sdcard/Download/termux/gba-metroidvania/brick_breaker
make clean && make
```
The output file is **`brick_breaker.gba`**, ready to play in mGBA, RetroArch, Pizza Boy GBA, or My Boy!
