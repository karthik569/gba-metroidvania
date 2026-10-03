# 🚀 SOLAR STRIKE: Void Viper 👾🎮

A high-octane, hardware-accelerated **Vertical Arcade Shoot 'Em Up (Shmup)** built from scratch in C and ARM assembly for the **Game Boy Advance (ARM7TDMI)**.

---

## 🕹️ Game Overview

* **Platform:** Game Boy Advance (ARM7TDMI @ 16.78 MHz)
* **ROM File:** `shmup.gba` (256 KB)
* **Framerate:** 60 FPS (hardware VBlank locked)
* **Resolution:** 240 × 160 pixels
* **Display Layout:** Authentic centered $160 \times 160$ vertical arcade cabinet playfield flanked by twin $40\text{px}$ cybernetic HUD sidebars (BG0)
* **Background Engine:** Dual-layer hardware parallax scrolling in GBA Mode 0:
  * **BG0:** Topmost cybernetic telemetry sidebars, lives indicators, radar, and text overlay.
  * **BG1:** Fast-scrolling foreground orbital scaffolding, space station corridors, and tumbling asteroid debris.
  * **BG2:** Slow-scrolling deep-space starfields, cosmic nebulae, and planetary horizons.
* **Audio Engine:** Direct PSG hardware chiptune sound engine with stereo speaker routing (`0xFF77`), driving stage themes, boss battle arpeggios, and explosive SFX.
* **SRAM Persistence:** Battery backup save system (`0x0E000000`) persisting High Score and Campaign Stage Progression (Stages 1–3) with 16-bit additive checksum verification.

---

## 🎮 Controller Layout

| Input | In-Flight Action | Title Screen Action |
| :--- | :--- | :--- |
| **D-Pad Left / Right** | 8-way directional flight with subpixel inertia & sprite banking | Cycles starting stage ($1 \dots \text{max\_unlocked}$) |
| **D-Pad Up / Down** | Pitch forward / pull back within active arcade corridor | — |
| **B Button / A Button** | Fire primary weapon (Pulse Vulcan, Twin Blaster, Triple Spread, Plasma Wave) | Launch / Start Mission |
| **R Shoulder** | Launch Screen-Clearing **Nova Bomb** (vaporizes bullets & deals massive area damage) | — |
| **Start Button** | Toggles in-game pause | Starts game from Title Screen |

---

## ⚡ Player Ship Mechanics & Weapons

### Precision Hitbox & Durability
* **Micro-Hitbox:** The Void Viper starfighter features a centered $4 \times 4$ pixel core hitbox, allowing precision navigation through dense bullet curtains.
* **Energy Shield Buffer:** The ship is equipped with a visible energy shield aura that absorbs **1 direct hit** from enemy fire or collision. The next hit destroys the ship and deducts a life.
* **Respawn:** 120 frames (2.0 seconds) of invulnerability flashing upon respawn with automatic shield reactivation.
* **Reserve Lives:** Start with 3 lives; extra lives awarded at 50,000 and 100,000 points.

### Weapon Upgrade Progression
Destroying elite formation leaders or heavy gunships drops special power-up capsules:

| Capsule | Upgrade Name | Streams | Description & Firing Pattern |
| :---: | :--- | :---: | :--- |
| **[P]** (Green) | **Pulse Vulcan** (LVL 1) | 1 Stream | Single forward rapid-fire kinetic plasma bolts. |
| **[P]** (Green) | **Twin Blaster** (LVL 2) | 2 Streams | Parallel high-velocity laser beams. |
| **[P]** (Green) | **Triple Spread** (LVL 3) | 3 Streams | Forward laser + angled $20^\circ$ left/right spread streams. |
| **[P]** (Green) | **Plasma Wave** (LVL 4) | 5 Streams | Maximum firepower: wide penetrating plasma arcs annihilating swarms. |
| **[B]** (Red) | **Nova Bomb** | Screen-Wide | Awards +1 Nova Bomb (max 5). Clears all active enemy bullets into bonus points and inflicts 150 damage. |
| **[S]** (Blue) | **Shield Restore** | Passive | Instantly restores the ship's energy shield buffer if collapsed. |
| **[1UP]** (Gold) | **Extra Life** | Reserve | Awards +1 reserve starfighter and 2,000 bonus points. |

---

## 👾 Enemy Hierarchy & Formations

* **Scout Drone (10 HP):** Fast agile drones swooping across the playfield in sinusoidal formations, discharging targeted plasma bolts.
* **Kamikaze Interceptor (15 HP):** Aggressive dart craft performing high-speed diagonal dives directly toward your flight vector.
* **Armored Gunship (60 HP):** Heavy $32 \times 16$ hover platform that halts mid-screen to unleash radial 3-way bullet bursts.
* **Heavy Cruiser (150 HP):** Heavily armored mid-boss warship firing dual alternating turret barrages.

---

## 🛸 3-Stage Cinematic Campaign & Boss Battles

### 🌌 Stage 1: Orbital Perimeter
* **Mission:** Atmospheric space station re-entry over Earth's blue horizon; navigate falling space station wreckage and scout drone waves.
* **Boss: Aegis Dreadnought**
  * **Structure:** Command flagship protected by dual breakable wing turrets (60 HP each) and a heavily armored core (160 HP).
  * **Phase 1 (Shielded):** Wing turrets fire alternating twin bursts; core is invulnerable.
  * **Phase 2 (Exposed/Enraged):** Destroying both turrets shatters the armor; core unleashes wide 5-way radial spread curtains while oscillating horizontally.
  * **Reward:** Unlocks Stage 2 in Battery SRAM!

### ☄️ Stage 2: Asteroid Infiltration
* **Mission:** Navigating a dense tumbling asteroid belt over a crimson gas giant; dodge kinetic mines, gunship pairs, and interceptor squadrons.
* **Boss: Titan Crust Crusher**
  * **Structure:** Heavy excavation war-platform (240 HP core + dual 90 HP shield pods).
  * **Phase 1:** Orbiting magnetic defense pods deflecting frontal fire; deploys high-speed kinetic bursts.
  * **Phase 2:** Pods shattered into kinetic debris; core activates high-speed oscillation and rapid 5-way plasma barrages.
  * **Reward:** Unlocks Stage 3 in Battery SRAM!

### ⚡ Stage 3: Void Core Fortress
* **Mission:** High-density bullet hell gauntlet through the heart of the alien mother-fortress with pulsing purple conduit conduits and heavy cruisers.
* **Climax Boss: Mothership Core Nexus (Void Leviathan)**
  * **Structure:** Massive $32 \times 32$ multi-segmented flagship nucleus (360 HP core + dual 120 HP defense pods).
  * **Phase 1:** Intersecting bullet spirals and heavy plasma bolt bombardment.
  * **Phase 2 (Overload):** Desperation overload state: doubled oscillation amplitude, continuous bullet curtains, and screen-shaking plasma pulses!
  * **Grand Victory:** Defeating the Master Core triggers cascading screen-wide explosions, celebratory chiptune anthem, and final campaign ranking!

---

## 💾 SRAM Battery Save & Practice Selector

* **High Score:** Persists all-time highest record across all play sessions with 16-bit additive checksum verification (`SLST100`).
* **Title Screen Stage Select:** Use **D-Pad Left / Right** on the Title Screen to choose any unlocked mission ($1 \dots \text{max\_stage}$) for instant practice.

---

## 🛠️ Building & Running

```bash
cd /sdcard/Download/termux/gba-metroidvania/shmup
make clean && make
```

The resulting ROM is **`shmup.gba`** (256 KB), ready to play in **mGBA**, **Pizza Boy GBA**, **RetroArch**, or on real Game Boy Advance hardware via flashcarts (EverDrive-GBA / EZ-Flash Omega).
