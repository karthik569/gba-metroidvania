# 🧱 GBA Brick Breaker (Arkanoid DX Edition) 🎮

An action-packed, hardware-accelerated **Arkanoid-style Brick Breaker** built from scratch for the **Game Boy Advance (ARM7TDMI)**.

---

## 🕹️ Game Overview

* **Platform:** Game Boy Advance (ARM7TDMI @ 16.78 MHz)
* **ROM File:** `brick_breaker.gba` (256 KB)
* **Framerate:** 60 FPS (hardware VBlank locked)
* **Resolution:** 240 × 160 pixels
* **Visual Engine:** Triple-zone dynamic background palette swaps (Deep Space, Cyber-Matrix & Neon Overdrive) + OAM hardware sprites + BG tilemap bottom floor barrier
* **Audio Engine:** Direct PSG hardware chiptune sound engine with 3 dedicated synth tracks (Neon Overdrive theme, dynamic multi-phase boss battle BGM, and Grand Master ending credits fanfare) + full suite of impact, warp, gravity, and laser SFX
* **SRAM Persistence:** Battery backup save system (`0x0E000000`) persisting High Score and 30-Stage Campaign Progression with Zone 1/2/3 completion flags, checksum validation, and legacy migration (supporting `BKBK102`, `BKBK101`, and `BKBK100`)

---

## 🎮 Controller Layout

| Input | Gameplay Action | Title Screen Action |
| :--- | :--- | :--- |
| **D-Pad Left / Right** | Smooth subpixel paddle steering | Cycles starting stage ($1 \dots \text{max\_unlocked}$) with Zone indicator |
| **A / B Button** | Launches docked/caught balls; Discharges twin Laser Blasters | Starts game at selected stage |
| **Start Button** | Toggles in-game pause | Starts game from Title Screen |

---

## 🌀 Quantum Warp Portals & Gravitational Physics (Zone 3)

Zone 3 introduces two game-changing physics mechanics:

### 1. Quantum Warp Portals (Portal Alpha & Beta)
* **Mechanic:** Linked pairs of sub-atomic quantum vortexes placed across the playfield (Portal Alpha in Cyan, Portal Beta in Magenta/Amber).
* **Teleportation:** Entering either portal instantly transports the ball to the twin portal exit $(X_B, Y_B) + \hat{v} \cdot 12\text{px}$, preserving trajectory while applying a **10% chromatic velocity boost**!
* **Cooldown Buffer:** A 24-frame ball-specific cooldown prevents jitter or re-entry looping.
* **Tactical Shortcut:** Enables players to bypass impenetrable Gold barriers and infiltrate high-value inner brick vaults.

### 2. Gravitational Singularity Wells
* **Mechanic:** Pulsing cyber-singularity cores that dynamically apply an inverse-distance gravitational pull ($\vec{F} \propto \frac{1}{r}$) within a 48px radius.
* **Orbital Slingshot:** Balls traveling near the well experience realistic curved trajectories, allowing skilled players to slingshot balls around gold bastions and hit obscured angles.
* **Dual Vortexes:** Stages such as Stage 25 feature twin counter-pull singularities that bend balls in complex S-curve figure-eights.

---

## ⚡ Power-Up Capsules

Destroying colored and armored bricks has a chance to drop special capsules:

| Capsule | Name | Color | Effect |
| :---: | :--- | :---: | :--- |
| **[W]** | **Wide Paddle** | Cyan | Expands paddle width from 32px to 48px for easier ball saves. |
| **[L]** | **Laser Blaster** | Red | Mounts twin laser cannons on paddle tips (press A/B to blast bricks & bosses). Stacks with active paddle size! |
| **[M]** | **Multi-Ball** | Green | Splits the active ball into 3 independently bouncing balls! |
| **[C]** | **Catch** | Amber | Magnetic paddle surface that catches returning balls for aimed re-launch. |
| **[S]** | **Slow Ball** | Blue | Temporarily slows down high-velocity balls for precision control. |
| **[P]** | **Extra Life** | Gold | Awards +1 reserve paddle and 1,000 bonus points. |
| **[B]** | **Energy Shield Barrier** | Amber | Deploys a 2-hit floor barrier across the bottom playfield via BG tilemaps (zero OAM overhead), catching missed balls and degrading on impact. |
| **[M]** | **Mega Piercing Ball** | Fiery Red | Imbues balls with plasma fury for 12 seconds, plowing straight through destructible bricks without deflecting and neutralizing enemy bolts and mines! |

---

## 🧱 Brick Varieties

### Classic Bricks
* **Red Brick:** 100 points (1 hit)
* **Blue Brick:** 120 points (1 hit)
* **Green Brick:** 150 points (1 hit)
* **Yellow Brick:** 180 points (1 hit)
* **Magenta Brick:** 200 points (1 hit)
* **Cyan Brick:** 250 points (1 hit)
* **Silver Armored Brick:** 500 points (2 hits, cracks on first impact)
* **Gold Barrier Brick:** Indestructible obstacle block (deflects balls & lasers)

### Interactive & Dynamic Bricks
* **Explosive (TNT) Brick (`BRK_TNT`):** Struck by a ball or laser, it detonates in a $3 \times 3$ area blast, destroying all 8 neighboring destructible bricks and cascading into adjacent TNT bricks. Sparing indestructible Gold barriers, it yields 150 points and rolls at most 1 power-up capsule per cascade.
* **Regenerating Bio-Matrix Brick (`BRK_REGEN`):** 3-hit self-repairing matrix. Each impact degrades its core (`BRICK_REGEN` $\rightarrow$ `DMG1` $\rightarrow$ `DMG2`). If left undisturbed for 5.0 seconds (300 frames), it repairs 1 damage level. Shattering it 3 times permanently destroys it, awarding 300 points and a capsule drop roll.
* **Kinetic Moving Patrol Brick:** Floating $16 \times 8$ armored drone bricks rendered via GBA OAM hardware sprites that patrol horizontally across the playfield at smooth 60 FPS, deflecting balls with a metallic ping and breaking after 2 direct hits.

---

## 👾 Boss Encounters

### Sector 1 Guardian (Stage 10 Boss)
* **Structure:** Central armored core (12 HP) shielded by two orbiting defense pods (4 HP each).
* **Defensive Phase:** The pods deflect frontal impacts; both pods must be neutralized before the core becomes vulnerable to ball and laser damage.
* **Attack Pattern:** Descends sinusoidal waves across the upper playfield while discharging plasma bolts downwards every 120 frames.
* **Enraged Phase:** Upon dropping to $\le 50\%$ HP (6 HP), oscillation frequency accelerates and plasma bolt discharge doubles to every 60 frames.
* **Victory Reward:** Defeating the Guardian unlocks **Zone 2 (Cyber-Matrix)** in battery SRAM!

### Master AI Core (Stage 20 Climax Boss)
* **Structure:** Massive $32 \times 32$ cybernetic boss core (18 HP) guarded by heavy satellite defense pods (6 HP each).
* **Defensive Shield:** Pods hover beside the core, blocking shots until shattered.
* **Aggressive Pattern:** Rapid sinusoidal hover and plasma bolt bombardment every 90 frames.
* **Enraged Phase:** At $\le 50\%$ HP (9 HP), the core enters an enraged overload state: movement sweep widens and bolt fire interval drops to 60 frames.
* **Victory Reward:** Shattering the Master AI Core unlocks **Zone 3 (Neon Overdrive)** in battery SRAM!

### The AI Overlord Core (Stage 30 Final Boss) 🤖🔥
* **Structure:** Colossal 3-phase multi-part final boss (24 HP) commanding orbital quantum satellites, laser-dampening armor, and gravitational distortion fields!
* **Phase 1: Quantum Satellites & Force Field:**
  * The Core is invulnerable behind a shimmering force field.
  * Protected by two orbiting **Quantum Satellites** (8 HP each) that dynamically sweep across the playfield.
  * Destroying both satellites shatters the force field with a massive concussive blast and screen banner!
* **Phase 2: Active Gravitational Singularity & Twin Lasers:**
  * The exposed Core activates an internal **Gravitational Singularity Well**, curving balls toward it while firing twin seeking laser blasters at the paddle.
* **Phase 3: Hyper Meltdown Overdrive:**
  * At $\le 50\%$ HP (12 HP), the Core enters meltdown! The chassis flashes blazing crimson/gold, speed accelerates, and the core unleashes 3-way spread laser bolts while dropping falling **Cyber-Mines** that detonate on contact with the paddle!
* **Grand Master Victory:**
  * Shattering the AI Overlord Core triggers a 120-frame cascading core detonation sequence, screen-shaking explosion fanfare, and the **Grand Master Cyber Breaker** ending screen!

---

## 🗺️ 30-Stage Progressive Campaign

The campaign spans three distinct visual sectors with dynamic 16-color background palette remapping:

### Zone 1: Deep Space Sector (Stages 1–10)
1. **Stage 1 (Initiation Grid):** Classic alternating colored rows; baseline paddle training.
2. **Stage 2 (Space Invader):** Retro 8-bit invader pattern testing angle rebounds.
3. **Stage 3 (Checkerboard Vault):** Alternating silver armor bricks and high-value gems.
4. **Stage 4 (Diamond Fortress):** Diamond armor perimeter encasing core points.
5. **Stage 5 (Golden Gauntlet):** Indestructible Gold pillars channeling high-speed ricochets.
6. **Stage 6 (Demolition Depot):** Introduction of **TNT Bricks** with cluster chain reactions.
7. **Stage 7 (Matrix Chamber):** Introduction of **3-Hit Regenerating Bricks** testing burst focus.
8. **Stage 8 (Orbital Perimeter):** Introduction of **Kinetic Moving Bricks** patrolling the mid-field.
9. **Stage 9 (Hazard Core):** Combined gauntlet: TNT caches, Gold walls, and wandering Hazard Drones.
10. **Stage 10 (The Apex Fortress / Sector 1 Guardian):** Dual-pod Boss encounter backed by kinetic shields and regenerating cores.

### Zone 2: Cyber-Matrix Sector (Stages 11–20)
11. **Stage 11 (Cyber Protocol):** Alternating bit-streams and armor capacitors in the neon cyber sector.
12. **Stage 12 (Binary Maze):** Gold circuit channels and embedded TNT nodes requiring precision bounce angles.
13. **Stage 13 (Neon Firewall):** Double silver armor barrier shielding dual clusters of regenerating bio-matrix blocks.
14. **Stage 14 (Data Stream):** Twin-tier corridor pattern guarded by kinetic patrol bricks.
15. **Stage 15 (Circuit Cascade):** Dense interlocking TNT nodes separated by gold baffles; triggers massive chain detonations.
16. **Stage 16 (Quantum Core):** Triple regenerating clusters fortified inside silver armor isolation rings.
17. **Stage 17 (Logic Gate):** Channelized gold gates directing ball flow into high-speed ricochets.
18. **Stage 18 (Mainframe Citadel):** Concentric armor fortress perfectly suited for Mega Piercing Ball assaults.
19. **Stage 19 (Glitch Infiltration):** Extreme gauntlet combining TNT, regenerating cores, gold baffles, and speed hazards.
20. **Stage 20 (The Cyber Nexus / Master AI Core):** Final showdown against the Master AI Core and its satellite defense pods!

### Zone 3: Neon Overdrive Sector (Stages 21–30)
21. **Stage 21 (Warp Gateway):** Dual Quantum Warp Portals flanking a central diamond of Magenta and Cyan bricks.
22. **Stage 22 (Quantum Maze):** Vertical pillars of Gold bricks with portals acting as shortcuts behind the gold barriers.
23. **Stage 23 (Event Horizon):** Circular ring of Silver & Cyan bricks orbiting a central Gravitational Singularity Well.
24. **Stage 24 (Kinetic Overdrive):** Two high-speed horizontal moving Kinetic Bricks guarding upper TNT explosive caches.
25. **Stage 25 (The Twin Vortex):** Dual Gravity Wells on left and right sides forming an S-curve figure-8 slingshot field.
26. **Stage 26 (Self-Repair Citadel):** Dense wall of 3-hit self-repairing Regen Bricks with a portal leading behind them.
27. **Stage 27 (Warp & Singularity):** Both dual Quantum Warp Portals AND a central Gravity Well; portals fling balls into the singularity!
28. **Stage 28 (Minefield Run):** Central gravity well surrounded by volatile TNT bricks and moving kinetic drones.
29. **Stage 29 (The Citadel Gates):** The ultimate pre-boss gauntlet: multi-layered Gold, Silver, Regen, TNT, and Portals.
30. **Stage 30 (The AI Overlord Core):** Final Boss Chamber: 32x32 AI Core with orbiting Quantum Satellites and Meltdown Overdrive!

---

## 💾 SRAM Save & Stage Selector

The cartridge battery SRAM (`0x0E000000`) stores persistent save data with checksum verification:
* **High Score:** Persists your personal record across all play sessions.
* **Max Stage Unlocked:** Automatically saves your highest reached stage ($1 \dots 30$).
* **Zone Completion Flags:** Tracks completion of Zone 1 (`zone1_cleared`), Zone 2 (`zone2_cleared`), and Zone 3 (`zone3_cleared`).
* **Title Screen Stage Select:** Press **D-Pad Left / Right** on the Title Screen to choose any unlocked starting stage. The title screen displays the sector badge (`ZONE: SPACE` for Stages 1–10, `ZONE: CYBER` for Stages 11–20, `ZONE: NEON` for Stages 21–30).
* **Backwards Compatibility:** Legacy `BKBK100` and `BKBK101` save data are automatically detected and safely migrated to `BKBK102`.

---

## 🛠️ Building & Running

```bash
cd /sdcard/Download/termux/gba-metroidvania/brick_breaker
make clean && make
```

The resulting ROM is **`brick_breaker.gba`** (256 KB), ready to play in **mGBA**, **Pizza Boy GBA**, **RetroArch**, or on real Game Boy Advance hardware via flashcarts (EverDrive-GBA / EZ-Flash Omega).
