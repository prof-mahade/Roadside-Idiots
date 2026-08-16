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

## Verified gameplay/readability polish

Local UE 5.8 playtests have already verified:
- AI/racecraft improvement remains stable
- player engine/skid feedback is useful
- item economy is five banana slots + three rotten-egg slots
- rotten eggs enter normal player use
- traffic advance warning fires at useful time-to-contact values
- telemetry records race setup, finish, speed, overtakes, item use and competition gap
- direct damage-source telemetry reaches `unknown=0`
- repeated tests produce both wins and close losses, so there is no evidence justifying an AI difficulty rewrite
- engine and advance-horn volumes were raised modestly after user feedback

---

## Major presentation pass 1 — RUNTIME VERIFIED at `3fb92f2`

The user compiled and ran the first major presentation block in UE 5.8. Runtime verification observed:
- `RI WORLD LANDMARKS`
- `RI ROAD MARKINGS`
- five `RI RIVAL IDENTITY` hooks
- item balance
- complete playtest telemetry
- traffic warnings
- finish celebration

The race completed normally and telemetry still attributed damage cleanly (`unknown=0`).

### What pass 1 added
- start/finish gantry
- quarter-lap landmark gates
- expanded market / tea-stop cluster
- parked CNG-style silhouette
- bus-stop shelter and parked bus silhouette
- pond / green-field section
- broad-canopy framing trees
- continuous road edge lines
- dashed center markings
- visible checkered start/finish stripe
- first rival-personality color cue
- finish confetti

### Screenshot findings from pass 1
The runtime log passed, but the screenshots exposed two visual/engine issues that required correction:
1. the tall personality pennant/pole visually dominated the motorcycle and could appear to hang below it;
2. finish confetti enabled body physics while intentionally using `NoCollision`, producing many Unreal Message Log warnings about incompatible collision/physics settings.

These were not accepted merely because the hook verifier passed.

---

## CURRENT AUTONOMOUS BLOCK 2 — PENDING LOCAL UE 5.8 COMPILE / VISUAL CHECK

This block is deliberately presentation-heavy and remains isolated from frozen gameplay systems.

### A. Finish celebration warning fix
`URIFinishCelebrationSubsystem` no longer enables Chaos/body simulation on non-colliding confetti.

Confetti is now animated kinematically by the subsystem:
- launch velocity
- gravity-like fall
- flutter
- rotation
- timed cleanup
- `NoCollision`
- `SetSimulatePhysics(false)`

Expected result: animated celebration with no `CollisionEnabled`/physics warning spam.

### B. Rival identity refinement
The oversized flag/pole was removed.

AI personalities now use compact motorcycle body accents:
- two thin side strips
- one small rear badge
- LEECH — green/teal
- HOTHEAD — orange
- PETTY — purple
- GREMLIN — lime
- BRAWLER — red
- TRYHARD — blue

These remain non-colliding and do not modify AI personality logic.

### C. Civilian traffic visual polish
New `URITrafficVisualPolishSubsystem` attaches `NoCollision` detail meshes to existing traffic actors while leaving their authoritative impact volumes and movement untouched.

Presentation details include:
- dark windscreens / rear glass
- bumpers and grille
- mirrors and rear plate
- TAXI roof sign and side stripe
- CNG windscreen / canopy / front mask
- MICROBUS side windows and belt trim
- DELIVERY VAN side panels / roof strip
- small silhouette-specific roof details for SUNDAY DRIVER / LOST DRIVER

This is intended to make traffic read as vehicles rather than moving colored boxes.

### D. Barrier / corner readability
`URIRoadMarkingSubsystem` now also creates sparse instanced amber/white reflector markers along the visual barrier faces.

Purpose:
- break up long black wall surfaces
- improve curvature perception
- strengthen speed cues

They remain `NoCollision` and do not modify barrier collision geometry.

### E. Landmark world signage
New `URIWorldSignageSubsystem` uses Unreal built-in TextRender only; it adds:
- `ROADSIDE IDIOTS` on the start gantry
- `TEA STOP` at the market section
- `BUS STOP` at the bus-stop section

No external font path, paid asset, collision or navigation ownership is introduced.

### F. Runtime verifier improvement
`tools/verify_polish_runtime.ps1` now checks:
- landmark layer
- world signage
- road markings
- rival identity
- optional traffic visual detail hooks
- item / playtest telemetry
- finish celebration / traffic warnings when they occur

It also treats known presentation physics/collision warnings as a failure, including the warning pattern seen in the user's screenshot.

---

## Isolation rule for block 2

Do not touch these systems during this gate unless a reproducible regression appears:
- `ARIAIController`
- `ARIRacingLineFollower`
- `URIBikeMovementComponent`
- authoritative road collision floor
- checkpoint/lap/place rules
- traffic route/movement behavior
- traffic impact volumes / damage tuning

The new traffic subsystem is presentation-only; it does not replace `ARITrafficVehicle` ownership of movement/collision.

---

## Immediate gate — next justified user intervention

The next user action is justified only because the remaining unknowns are local compile/render/perception:
1. sync the latest `dev/mvp-foundation` head;
2. compile `RoadsideIdiotsEditor Win64 Development` under UE 5.8.1;
3. run a race with traffic enabled;
4. verify the old tall flag is gone and compact rival accents look integrated;
5. verify traffic visually reads better without any change in collision behavior;
6. verify barrier reflectors help rather than clutter;
7. verify `ROADSIDE IDIOTS`, `TEA STOP`, and `BUS STOP` signs face/read correctly;
8. finish the race and confirm the confetti still moves but the previous collision/physics Message Log warning is gone;
9. run `tools/verify_polish_runtime.ps1`;
10. confirm frozen bike / flat-road / AI behavior remains unchanged.

If compile fails, fix compile before making any visual judgment.

If one presentation element looks wrong, correct only that visual layer; do not reopen racing physics/AI.

---

## After block 2 passes

Continue autonomously with the highest-value remaining Demo 1 polish:

### Asset-first art/audio replacement
- replace synthetic fallback sounds with verified free/custom assets where practical
- prioritize engine/load, impact, countdown/GO/lap/finish, horn and item feedback
- never guess local asset paths

### Environment art replacement
- gradually replace blockout silhouettes with verified free local vegetation/props
- preserve Bangladesh/South-Asian roadside identity
- keep decorative content outside authoritative road collision

### Player-test build
- produce a fresh standalone Shipping package after the presentation gate
- run package preflight and runtime verification
- use `docs/PLAYER_TEST_PLAN.md` with outside players

### Demo 2 only after player-test feedback
Potential themes:
- another route/environment variation
- new race/hazard variation
- stronger rival identity
- deeper accessibility/remapping
- progression through side-grades/cosmetics
- multiplayer/social work much later

---

## Known acceptable Demo 1 limitations

Not current blockers:
- traffic can still collide in dense conditions
- physics are arcade/prototype rather than commercial motorcycle simulation
- environment/art/audio remain prototype quality
- one main course/mode is acceptable for Demo 1
- multiplayer is deferred
