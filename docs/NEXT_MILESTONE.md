# Next milestone — Demo 1 Major Presentation Polish Gate

## Demo 1 status — FUNCTIONALLY COMPLETE

The standalone Windows demo has already been packaged and played outside Unreal Editor. The accepted functional baseline includes:
- configurable 2–6 AI opponents
- selectable 1–5 laps and 0–6 traffic
- CLEAN / BALANCED / MAYHEM race-chaos selection
- stable countdown / lap / place / finish / restart flow
- motorcycle + Manny presentation
- stable Pure-Pursuit racing AI without the old recurring wall oscillation
- accepted professional-pace / predictive-overtaking racecraft pass
- slap / banana peel / rotten egg / dog-poop / cow-patty loops
- civilian traffic
- minimap and race HUD
- title/setup/pause/settings/restart/quit flow
- free/custom-content-only packaging policy

Do not reopen solved foundation systems merely because they could be more sophisticated.

---

## Product north star

Permanent fantasy:

> **You are a competent motorcycle rider trying to win while surrounded by idiots.**

The game is **drive-first, chaos-second**. The player, camera, controls and road-following AI must not feel more foolish than the characters.

The next goal is **more readable, more distinctive and more replayable**, not “more systems at any cost.”

---

## Permanent constraint — FREE ONLY

Roadside Idiots may use only:
- content/tools/assets available to the user for $0 under the applicable license, or
- content created by this project.

The removed SankoolArts / `CompoundWall_Kit` content must never return. Packaging preflight blocks it.

---

## Frozen foundation

Do not retune/rewrite without a reproducible regression:
- physical bike movement / physics baseline
- continuous flat authoritative road collision floor
- `ARIRacingLineFollower` Pure Pursuit + accepted racecraft stack
- checkpoint/lap/place/finish rules
- core camera baseline
- assisted egg targeting
- basic road dimensions / oval route geometry

`05c2604` remains the accepted AI/racecraft reference state.

---

## Verified polish before the current major batch

The user has already verified these in local UE 5.8 playtests:
- AI/racecraft improvement remained stable
- player engine/skid feedback was acceptable
- item economy was corrected to five banana slots + three rotten-egg slots
- rotten eggs entered normal player use (`egg_pickups` and `egg_uses` verified)
- traffic advance warning fired at useful time-to-contact values
- passive telemetry records race setup, finish, speed, overtakes, item use and competition gap
- direct damage-source telemetry now reaches `unknown=0` in the latest test
- repeated tests produced both wins and a close 2nd-place result, so there is no evidence justifying an AI difficulty rewrite

The user then requested slightly louder engine feedback and slightly louder advance traffic horn. Those volume increases are present in the current branch and should be judged together with the major presentation batch.

---

## CURRENT MAJOR AUTONOMOUS BATCH — PENDING LOCAL UE 5.8 VERIFICATION

This batch intentionally makes a larger visible improvement before asking the user to test again.

### 1. Distinct roadside landmark zones

New `URIRoadsideLandmarkSubsystem` adds a presentation-only second landmark layer using Engine basic shapes:
- start/finish gantry
- stronger quarter-lap landmark gates
- expanded roadside market / tea-stop cluster
- parked CNG-style silhouette near the market
- bus-stop shelter and parked local-bus silhouette
- pond / green-field section
- broad-canopy framing trees

These landmarks wait until the configured race world exists and then spawn once.

All pieces explicitly disable collision and do not own AI, checkpoints, road geometry or race state.

### 2. Road markings / speed readability

New `URIRoadMarkingSubsystem` builds markings with instanced static meshes:
- continuous outer edge lines
- regularly spaced center dashes
- visible two-row checkered start/finish stripe

The markings sit slightly above the visual road, use `NoCollision`, disable navigation influence, and must never become an authoritative road surface. This is specifically designed not to recreate the old segmented-road bump problem.

### 3. Finish celebration

New `URIFinishCelebrationSubsystem` observes the human finish state and creates a short-lived confetti burst:
- presentation only
- no finish-audio ownership (existing presentation subsystem remains audio owner)
- confetti collision is disabled
- confetti physics affects only the confetti pieces
- pieces clean themselves up after a few seconds

### 4. Rival personality visual identity

New `URIRivalIdentitySubsystem` gives AI bikes a small color-coded personality pennant:
- LEECH — green/teal
- HOTHEAD — orange
- PETTY — purple
- GREMLIN — lime
- BRAWLER — red
- TRYHARD — blue

The pennants are non-colliding and do not modify personality logic or AI behavior. They exist so players can recognize recurring rivals before reading a HUD label.

### 5. Runtime verification tool

`tools/verify_polish_runtime.ps1` summarizes the newest Unreal log and checks for:
- `RI WORLD LANDMARKS`
- `RI ROAD MARKINGS`
- `RI RIVAL IDENTITY`
- `RI ITEMS BALANCE`
- `RI PLAYTEST START`
- optional finish / damage / competition / traffic / celebration lines

This replaces repetitive manual log hunting during the next verification pass.

### Isolation check

The presentation batch was compared against the previously tested `8e86182` state. The major batch adds new presentation/world files only; it does **not** modify:
- `ARIAIController`
- `ARIRacingLineFollower`
- `URIBikeMovementComponent`
- authoritative road builder / collision floor
- checkpoints / race rules
- traffic driving behavior

---

## Immediate gate — this is the next justified user intervention

The next user action is needed because the remaining uncertainty is local compile/render/player perception, not source reasoning.

Required checks:
1. Sync latest `dev/mvp-foundation`.
2. Compile `RoadsideIdiotsEditor Win64 Development` on UE 5.8.1.
3. Launch the editor and run a normal race.
4. Confirm the road markings are visible and do not create bumps/collision.
5. Confirm the start/finish gantry and new market/bus-stop/pond sections make the lap feel more distinctive rather than cluttered.
6. Confirm AI pennants are visible/useful and do not obscure riders.
7. Confirm finish confetti appears once and never pushes the bike.
8. Confirm the slightly louder engine and traffic-warning horn now feel right.
9. Run `tools/verify_polish_runtime.ps1` and inspect the summary.
10. Briefly watch for any regression in the frozen AI/wall behavior.

If the compile fails, fix the compile error before judging visuals.

If the batch compiles but one visual element is ugly/obstructive, make a presentation-only correction; do not reopen racing physics/AI.

---

## Next autonomous work after this gate

If the major batch is accepted:

### A. Asset-first art/audio replacement
- replace the most synthetic fallback sounds with suitable free/custom assets when practical
- prioritize engine/load, impact, countdown/GO/lap/finish, horn and item feedback
- use known local free assets only when their package paths are verified; never guess asset paths

### B. Rival identity / chaos readability refinement
- keep personalities visually and behaviorally memorable
- prefer blocking, traps, opportunism and grudges over constant slapping
- preserve low-level driving competence

### C. Environment art pass
- use verified free local vegetation / props where package paths are known
- keep Bangladesh/South-Asian roadside identity affectionate and recognizable
- replace blockout silhouettes gradually rather than adding collision-heavy decorative packs

### D. Demo 2 only after the polish gate
Possible themes:
- another route/environment variation
- stronger rival identity
- new race/hazard variation
- progression through side-grades/cosmetics rather than grind
- deeper accessibility/remapping
- social/multiplayer features much later

---

## Known acceptable Demo 1 limitations

Not current blockers:
- traffic can still collide in dense conditions
- physics are arcade/prototype rather than commercial motorcycle simulation
- environment/art/audio remain prototype quality
- one main course/mode is acceptable for Demo 1
- multiplayer is deferred
