# 🤖 IRON PROTOCOL: Micro-Tactics ⚔️🎮

A tactical, hardware-accelerated **Turn-Based Strategy Game** built from scratch in C and ARM assembly for the **Game Boy Advance (ARM7TDMI)**.

---

## 🕹️ Game Overview

* **Platform:** Game Boy Advance (ARM7TDMI @ 16.78 MHz)
* **ROM File:** `tactics.gba` (256 KB)
* **Framerate:** 60 FPS (hardware VBlank locked)
* **Resolution:** 240 × 160 pixels
* **Battlefield Dimensions:** $20 \times 15$ tile grid ($320 \times 240\text{px}$) with smooth subpixel hardware camera auto-tracking
* **Display Engine:** 3-layer GBA Mode 0 architecture:
  * **BG0:** Topmost tactical UI, turn counter banner, action dialogs, and combat forecasts.
  * **BG1:** Movement & Attack reach highlight grid (Blue for movement, Red for attack targets).
  * **BG2:** $20 \times 15$ detailed battlefield terrain map (Plains, Forests, Mountains, Rivers, Bridges, Cities, HQs).
* **Audio Engine:** Direct PSG hardware chiptune sound engine with stereo speaker routing (`0xFF77`), dynamic Player Phase march, Enemy Phase cadence, CO power themes, and combat SFX.
* **SRAM Persistence:** Battery backup save system (`0x0E000000`) persisting Campaign Progression (Missions 1–3), Career Score, and S/A/B/C Tactical Ranks.

---

## 🎮 Controller Layout

| Input | In-Battle Action | Title Screen Action |
| :--- | :--- | :--- |
| **D-Pad** | Navigates tactical selection cursor / camera auto-pans near screen borders | Cycles mission selection ($1 \dots \text{max\_unlocked}$) |
| **A Button** | Selects unit / Confirms movement destination / Executes attack / Confirms action | Launch / Start Mission |
| **B Button** | Deselects unit / Cancels move and returns unit to starting tile / Dismisses menus | — |
| **Start Button** | Opens Field Menu (End Turn / CO Power) when cursor is on empty tile | Starts mission from Title Screen |

---

## 🗺️ Terrain Defense & Movement Matrix

| Terrain | Move Cost (Mech / Wheels / Hover / Air) | Defense Cover | Tactical Properties |
| :--- | :---: | :---: | :--- |
| **Plains** | 1 / 2 / 1 / 1 | **10%** | Standard open grassland terrain. |
| **Forest** | 1 / 3 / 2 / 1 | **20%** | Heavy pine canopy providing ambush cover for Mechs. |
| **Mountain** | 2 / — / — / 1 | **40%** | Impassable to wheeled/tracked armor; supreme defensive vantage for Walkers. |
| **River** | 2 / — / 1 / 1 | **0%** | Slow wading for mechs; Hover Tanks glide across effortlessly. |
| **Road / Bridge** | 1 / 1 / 1 / 1 | **0%** | High-speed transit route for rapid armor repositioning. |
| **City Ruins** | 1 / 1 / 1 / 1 | **30%** | Capturable installation; fortifies defenders. |
| **Player HQ** | 1 / 1 / 1 / 1 | **40%** | Allied command base; defend at all costs! |
| **Enemy HQ** | 1 / 1 / 1 / 1 | **40%** | Capturing with an Assault Walker triggers instant mission victory! |

---

## 🤖 Combined Arms Unit Classes & Matchups

All units have 10 HP represented by corner badges on damaged units:

| Unit Class | Sprite Icon | Move Points | Range | Primary Role & Matchup Strengths |
| :--- | :---: | :---: | :---: | :--- |
| **Assault Walker** | Bipedal Mech | 4 (Mech Legs) | 1 | Versatile all-terrain frontline fighter. Can capture Cities and enemy HQs. Strong vs Recons and Walkers. |
| **Recon Drone** | Wheeled Buggy | 6 (Wheels) | 1 | Fast scout. Devastating against infantry and mechs; vulnerable to heavy tanks. |
| **Hover Tank** | Heavy Hover Armor | 5 (Hover) | 1 | 75mm main battle cannon. Glides over rivers and plains; crushes light units. |
| **Siege Artillery** | Missile Rig | 3 (Tracks) | 2–3 (Indirect) | Long-range rocket bombardment. Cannot move and fire on same turn. Attacks without receiving counter-attacks! |
| **VTOL Gunship** | Flying Gunship | 6 (Airborne) | 1 | Ignores all terrain obstacles. High-mobility tank killer with anti-armor rockets. |

### Damage & Counter-Attack Formula
$$\text{Damage} = \frac{\text{BaseMatchup}(\text{Attacker}, \text{Defender}) \times \text{AttackerHP} \times (100 - \text{TerrainDef})}{1000}$$
* **Counter-Attacks:** If the defender survives a direct attack (Range 1) and is not disabled by an EMP strike, the defender immediately counter-attacks with its remaining HP!

---

## ⚡ Commander (CO) Super Powers

Dealing and taking combat damage charges your 4-star **CO Meter**:

* **Commander Ryan (Player)**:
  * **"ORBITAL EMP STRIKE"**: Calls down an orbital kinetic strike on an enemy cluster, dealing 3 direct damage and disabling counter-attacks for 1 turn!
  * **"OVERDRIVE BLITZ"**: Grants all deployed allied units **+2 Movement Points** and **+30% Attack Power** for the current turn.
* **General Vex (Enemy AI)**:
  * **"SIEGE PROTOCOL"**: Grants +1 Range to all indirect artillery and boosts direct armor firepower by +20%.

---

## 🎖️ 3-Mission Campaign

1. **Mission 1 (River Bridgehead):**
   * *Forces:* 2 Walkers, 1 Recon, 1 Tank vs 2 Walkers, 2 Recons, 1 Tank.
   * *Objective:* Secure the central river crossing, capture the comm relay city, and capture the Eastern HQ or rout all enemies.
2. **Mission 2 (Canyon Artillery Ambush):**
   * *Forces:* 2 Walkers, 2 Tanks, 1 Artillery vs 3 Tanks, 2 Artillery, 2 Walkers.
   * *Objective:* Navigate a narrow mountain canyon pass guarded by fortified enemy artillery batteries.
3. **Mission 3 (The Iron Citadel):**
   * *Forces:* Full combined arms (Walkers, Tanks, Artillery, VTOL Gunship) vs Fortified Cyber Citadel Bunkers, Artillery, and Vex's Flagship Walker.
   * *Objective:* Breach the citadel walls and capture General Vex's HQ for Grand Campaign Victory!

### Mission Performance Ranking
Missions are evaluated upon completion based on Speed (turns taken), Power (enemies destroyed), and Technique (allies preserved):
* **S-Rank:** $\le 10$ turns (Master Tactician)
* **A-Rank:** $\le 15$ turns (Superior Commander)
* **B-Rank:** $\le 20$ turns (Combat Veteran)
* **C-Rank:** $> 20$ turns (Mission Qualified)

---

## 💾 Battery SRAM Save System

* **Cartridge Battery SRAM (`0x0E000000`)** automatically saves your highest reached mission ($1 \dots 3$), tactical ranks (S/A/B/C), and total career score with 16-bit additive checksum verification (`IRNT100`).
* Select any unlocked mission on the **Title Screen** using **D-Pad Left / Right**.

---

## 🛠️ Building & Running

```bash
cd /sdcard/Download/termux/gba-metroidvania/tactics
make clean && make
```

The compiled binary is **`tactics.gba`** (256 KB), ready to play in **mGBA**, **Pizza Boy GBA**, **RetroArch**, or on real Game Boy Advance hardware via flashcarts (EverDrive-GBA / EZ-Flash Omega).
