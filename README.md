# AERO-VOID: Outpost Zero 🚀🎮

**A Sci-Fi Metroidvania homebrew action-platformer engineered for the Game Boy Advance (GBA).**

---

## 🕹️ Game Overview

* **Platform:** Game Boy Advance (ARM7TDMI @ 16.78 MHz)
* **ROM File:** `outpost_zero.gba` (256 KB)
* **Resolution:** 240 × 160 pixels @ 60 FPS
* **Genre:** Sci-Fi Metroidvania
* **Engine:** Custom zero-overhead C/ASM GBA Engine

Traverse the subterranean labyrinth of **Outpost Zero**, an abandoned deep-space research installation infested with bio-mechanical alien hazards. Discover weapons, upgrade into compact drone configurations to breach tight crawlspaces, blast through reinforced security doors, and dismantle the rogue **Corrupted Defense Sentinel**.

---

## 🎮 Controller Layout

| Input | Action | Behavior |
| :--- | :--- | :--- |
| **D-Pad Left / Right** | Move / Run | Smooth momentum and friction physics |
| **D-Pad Down** | Crouch / Nano-Drone | Deploys spherical 1-tile rolling drone for narrow ducts |
| **D-Pad Up** | Aim Upward / Unmorph | Aims cannon vertically (or unrolls drone if clearance exists) |
| **A Button** | Jump & Wall-Jump | Variable jump height; kick off vertical walls to ascend shafts |
| **B Button** | Arm Cannon Fire | Discharges energy blaster beams (or Missiles if toggled) |
| **R Shoulder** | Missile Swap | Toggles Concussion Missiles (destroys Red Security Barriers) |
| **L Shoulder** | Diagonal Lock | Locks aim vector diagonally while moving |

---

## 🗺️ World Map & Progression (Vertical Slice)

```
[R0: Landing Dock] <---> [R1: Airflow Shaft] <---> [R2: Security Armory (Missiles)]
                                |
                        [R3: Power Conduit (Morph Duct)]
                                |
                        [R4: Hub Corridor] <---> [R5: Sentinel Boss Arena]
                                |
                        [R6: Save Station]
```

1. **Room 0 (Landing Dock):** Awakening chamber with introductory platforming and crawler enemies.
2. **Room 1 (Airflow Shaft):** Vertical climbing gauntlet testing wall-kick mechanics.
3. **Room 2 (Security Armory):** Contains the **Concussion Missile** upgrade pod.
4. **Room 3 (Power Conduit):** 1-tile narrow energy tunnel requiring Nano-Drone morphing to bypass acid pools.
5. **Room 4 (Hub Corridor):** Blocked by a Red Security Barrier block (requires Missiles to shatter).
6. **Room 5 (Boss Arena):** Clash with the *Corrupted Defense Sentinel* (32x32 boss with sinusoidal hover and laser attacks).
7. **Room 6 (Save Station):** Computer terminal that completely recharges Energy and Missiles.

---

## 🛠️ Building the ROM

### Prerequisites
Inside Ubuntu PRoot:
```bash
apt-get install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi make python3
```

### Compiling
```bash
cd /sdcard/Download/termux/gba-metroidvania
make clean && make
```
The output file is **`outpost_zero.gba`**, ready to be opened in any GBA emulator (RetroArch, Pizza Boy GBA, mGBA, My Boy!).

---

## 📁 Source Code Structure

* `src/main.c`: 60 FPS hardware-synced game loop and state manager.
* `src/crt0.s`: GBA ARM cartridge header, interrupt stack configuration, and memory copy routines.
* `src/graphics.c`: Hardware VRAM, OAM sprite attribute manager, and high-speed DMA3 transfers.
* `src/input.c`: Hardware keypad poller (`KEY_A`, `KEY_B`, `KEY_R`, `KEY_L`, D-Pad).
* `src/audio.c`: GBA PSG direct-register sound engine (beam lasers, explosions, jump chirps, boss roar).
* `src/map.c`: 7-room tilemap matrix and collision query system.
* `src/entity.c`: Fixed-point subpixel physics (8.8 FP), player mechanics, enemy AI, and weapons.
* `src/assets.c`: 4bpp 16-color indexed background tiles and sprite sheets.
* `gba_cart.ld`: GNU linker script mapping code to ROM (`0x08000000`) and data to EWRAM (`0x02000000`).
* `tools/fix_header.py`: GBA header complement checksum verification tool.
