# GRAND SLAM: Advance Tennis 🎾🏆

**A Hardware-Accelerated Elevated-Perspective Tennis Game for the Game Boy Advance (ARM7TDMI)**

---

## 1. Overview & Vision

**GRAND SLAM: Advance Tennis** brings authentic, responsive tennis action to the Game Boy Advance, inspired by handheld classics like *Mario Tennis: Power Tour* and *Virtua Tennis*. Featuring true **3D parabolic ball arc physics** ($X, Y, Z$ altitude coordinates with ground drop shadows), full racket shot mechanics (Topspin, Slice, Lob, Drop Shot, Overhead Smash, and 2-tap Ace Serves), three authentic court surfaces (Grass, Clay, Hard Court), complete tennis scoring rules with line judgment, 3 game modes, intelligent AI opponents, and cartridge battery SRAM persistence.

---

## 2. Controls & Shot Catalog (Mario Tennis Mechanics)

| Input Combination | Action / Shot Type | In-Game Dynamics & Tactical Use |
| :--- | :--- | :--- |
| **D-Pad** | Court Movement & Shot Aiming | Move across baseline and net; steer ball crosscourt or down-the-line during racket swings |
| **Hold A** | **Topspin Drive (Charge)** | Charge power while moving; release / contact unleashes deep, fast baseline drive with Red/Yellow fire trail |
| **Hold B** | **Slice / Backspin (Charge)** | Low-altitude skid bounce with Blue trail; disrupts opponent's timing, deadly on Grass |
| **Hold A + B** | **Defensive Lob (Charge)** | High altitude parabolic arc ($Z \ge 60$); escapes corner pressure and punishes net rushers |
| **Down + B** | **Drop Shot** | Soft touch dying just past the net cord; punishes opponents camping deep |
| **High Ball + A** | **Overhead Smash** | Blazing 125 MPH downward spike with screen shake; instant winner on floaters |
| **Serve Toss + A/B** | **Power Ace Serve** | Tap `A` or `B` to toss ball upward; strike at **peak altitude** ($Z \ge 26$) for a 125–135 MPH power ace! |
| **Start / A Button** | Select / Advance | Advance through menus, service setups, and game over screens |

---

## 3. 3D Ball Physics & Court Drop Shadow

```
           (Ball Sprite - Scaled by Altitude Z)
                      ( O )
                        |
                        |  <-- Visual Altitude Z
                        |
                     ( ooo )
         (Court Drop Shadow on Surface Ground)
```

The tennis ball operates in a continuous 3D coordinate space ($X, Y, Z$) governed by sub-pixel parabolic gravity ($g \approx 0.125\text{ px/frame}^2$):
- **Visual Depth Cue**: A dedicated ground drop shadow is rendered on the court surface directly beneath the ball. The vertical distance between the ball and its shadow provides instantaneous, intuitive timing for forehand, backhand, and overhead smash strikes!
- **Net Collision**: Crossing the net ($Y = 76$) requires $Z \ge 16\text{px}$. Striking the tape triggers net cord rattle audio and potential let dribbles!

---

## 4. Court Surfaces & Physics Modifiers

| Court Surface | Color Palette | Friction | Bounce Height ($V_z$) | Tactical Dynamics |
| :--- | :---: | :---: | :---: | :--- |
| **Grass Court** | Wimbledon Emerald | **Low** ($0.85\times$) | **Low** ($0.65\times$) | Fast skid pace; favors slice curves and aggressive serve-and-volley play |
| **Clay Court** | Roland Garros Red | **High** ($0.75\times$) | **High** ($0.85\times$) | Heavy friction slows forward drive; high topspin kick favors baseline rallies |
| **Hard Court** | US Open Cobalt Blue | **Medium** ($0.80\times$) | **Medium** ($0.75\times$) | Balanced, true bounce speed; standard all-round tournament battleground |

---

## 5. Game Modes & Tournament Structure

1. **Grand Slam Tournament Cup**:
   - Compete through a 3-round knockout bracket for the championship trophy:
     - **Quarter-Final**: *Leo "The Wall" Vance* (Defensive Baseliner, rarely makes unforced errors)
     - **Semi-Final**: *Kenji Sato* (Aggressive Serve-and-Volleyer, rushes net after every return)
     - **Grand Final**: *Marcus "Thunder" Thorne* (Power Server, 130+ MPH aces and heavy topspin)
   - Hoist the Grand Slam Trophy into your battery SRAM trophy cabinet!
2. **Quick Exhibition Match**:
   - Custom match setup: Choose Court Surface, AI Opponent, and start immediately.
3. **Target Practice Mini-Game**:
   - 60-second target drill challenge hitting bullseye rings on the far court to build combo scores.

---

## 6. Scoring Rules & In/Out Line Judgment

- **Standard Tennis Scoring**:
  - Points: `0 (Love)` $\rightarrow$ `15` $\rightarrow$ `30` $\rightarrow$ `40` $\rightarrow$ `Game`.
  - Tie at 40-40 triggers `Deuce`. Consecutive points lead to `Advantage` and `Game`.
  - Sets: First to 3 games wins the set and match!
- **Line Calls & Chalk Puffs**:
  - Exact geometric line evaluation on every court bounce:
    - Balls touching or inside lines trigger a **Chalk Puff VFX** and continue play.
    - Balls landing outside the lines trigger an immediate umpire call: `"OUT!"`.
  - Service Faults allow a Second Serve; consecutive faults trigger a `"DOUBLE FAULT!"`.

---

## 7. Technical Specifications

- **Platform**: Game Boy Advance (ARM7TDMI @ 16.78 MHz)
- **Display**: 240 × 160 pixels @ 59.73 Hz hardware VBlank synchronization
- **Graphics Engine**:
  - **Mode 0 (Text Mode)** with 3 active hardware background layers:
    - **BG0 (Priority 0)**: Non-scrolling scoreboard HUD, announcements, and serve speed radar (MPH).
    - **BG1 (Priority 1)**: Stadium grandstand with animated spectator crowd, umpire chair, net posts, and mesh.
    - **BG2 (Priority 2)**: Elevated perspective court lines and textured surfaces.
  - **128 Hardware Sprites (OAM)**: Near Player, Far Opponent, 3D Z-scaled Tennis Ball, Drop Shadow, Chalk Puffs, and Swing Trails.
- **Audio Synthesizer**: Direct GBA PSG stereo sound engine:
  - Channels 1 & 2: Menu Prelude, Match Rally Beat, and Trophy Ceremony Fanfare.
  - Channel 4: Racket thwacks, slices, court bounces, and crowd applause swells.
- **Save Persistence**: 32 KB Cartridge Battery SRAM at `0x0E000000` with magic header `"TENN100"`.

---

## 8. How to Build & Play

### Build
From project root:
```bash
make -C tennis clean && make -C tennis
```

Produces `tennis/tennis.gba` (262,144 bytes, complement checksum `0xD0`).

### Emulator
```bash
mgba-qt tennis/tennis.gba
```
