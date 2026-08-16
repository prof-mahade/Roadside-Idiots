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

## Major presentation pass 1 — VERIFIED at `3fb92f2`

Pass 1 established:
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

The first screenshot exposed two defects despite successful runtime hooks:
1. the tall personality flag/pole dominated the motorcycle;
2. physics-driven `NoCollision` confetti produced Unreal collision/physics warnings.

Those defects were corrected in pass 2 rather than accepted just because logging passed.

---

## Major presentation pass 2 — VERIFIED at `9148a6b`

The user compiled and ran pass 2 in UE 5.8. Runtime verification passed all required hooks:
- landmark world layer
- built-in world signage
- road markings + 40 reflectors
- five rival identity hooks
- four traffic visual-polish hooks
- item balance / playtest telemetry
- traffic warnings
- kinematic finish celebration
- known presentation physics/collision warning regression check

The previous Message Log warning spam was gone.

### Pass 2 accepted technical changes
- finish confetti is kinematic, non-colliding and warning-free
- oversized rival flag/pole was removed
- traffic gained first-pass windows/trim/mirrors/plates and vehicle-specific details
- visual barriers gained sparse reflectors
- `ROADSIDE IDIOTS`, `TEA STOP`, and `BUS STOP` use built-in TextRender

### Pass 2 screenshot findings
The second screenshot still showed several prototype-quality visual weaknesses:
- civilian sedan traffic still read primarily as a large rectangular body/cabin block
- rival body accents were much better than the old flag but still looked too saturated / detached below clustered bikes
- landmark text had no backing board and could wash out against the environment
- horizon remained mostly empty sky and flat ground beyond the near roadside props

These observations justify another presentation-only pass; they do not justify reopening gameplay systems.

---

## CURRENT AUTONOMOUS BLOCK 3 — PENDING LOCAL UE 5.8 COMPILE / VISUAL CHECK

Block 3 is based directly on the second screenshot and remains isolated from frozen gameplay.

### A. Civilian traffic layered-shell pass
`URITrafficVisualPolishSubsystem` now builds a clearer visible shell around existing traffic actors while leaving `ARITrafficVehicle` movement, impact volume and damage untouched.

New/expanded cues include:
- body-material-matched hood / trunk / roof layers for sedan-like traffic
- framed rear glass instead of one large gray cabin face
- side glass
- headlamps and tail lamps
- lower side sills
- bumpers / grille / plate / mirrors
- compact CNG rear lamps and side framing
- existing TAXI / MICROBUS / DELIVERY VAN / SUNDAY DRIVER / LOST DRIVER identity details

All added pieces use `NoCollision`, do not affect navigation, and do not cast their own shadows.

### B. Rival identity integration pass
The previous body strips were shrunk and moved inward toward the tail/fairing.

Changes:
- substantially smaller side accents
- smaller rear badge
- reduced saturation so accents do not compete with pickups/hazards
- no accent shadows
- log style now reports `integrated_tail_badges`

No AI personality behavior changed.

### C. Landmark sign backing boards
`URIWorldSignageSubsystem` now places thin dark backing boards behind:
- `ROADSIDE IDIOTS`
- `TEA STOP`
- `BUS STOP`

The boards use Engine cube/material assets only and remain collision/navigation/shadow free.

### D. Distant roadside backdrop
New `URIRoadsideBackdropSubsystem` fills the empty horizon efficiently using instanced Engine shapes:
- muted low-rise building belt on the more built-up half of the lap
- occasional rooftop water-tank silhouettes
- broad distant tree belt on the rural half

The backdrop is intentionally low-detail, low-contrast, non-colliding, non-navigable and well outside the authoritative race corridor.

Expected runtime hook:
`RI WORLD BACKDROP buildings=22 trees=30 tanks=... collision=off navigation=off`

### E. Verification tooling
`tools/verify_polish_runtime.ps1` now requires the distant-backdrop hook in addition to the existing landmark/signage/road/rival/item/playtest checks and warning regression scan.

---

## Block 3 isolation check

Compared with the user-verified `9148a6b` state, Block 3 changes presentation/tooling only:
- `RIRivalIdentitySubsystem`
- `RITrafficVisualPolishSubsystem`
- new `RIRoadsideBackdropSubsystem`
- `RIWorldSignageSubsystem`
- runtime verifier
- this milestone document

Do **not** touch during this gate unless a reproducible regression appears:
- `ARIAIController`
- `ARIRacingLineFollower`
- `URIBikeMovementComponent`
- authoritative road collision floor
- checkpoint/lap/place rules
- traffic route/movement behavior
- traffic impact volumes / damage tuning

---

## Immediate gate — next justified user intervention

The next user action is justified because the remaining uncertainty is local compile/render/perception:
1. sync latest `dev/mvp-foundation`;
2. compile `RoadsideIdiotsEditor Win64 Development` under UE 5.8.1;
3. run a race with 4+ traffic;
4. judge whether ordinary cars finally read less like boxes, especially from the rear;
5. confirm rival colors now feel integrated rather than neon/detached;
6. confirm the three landmark signs are easier to read against their backing boards;
7. confirm the distant skyline/tree belt improves the empty horizon without making the track cluttered;
8. confirm flat-road / bike / AI / traffic collision behavior is unchanged;
9. finish the race and run `tools/verify_polish_runtime.ps1`;
10. watch for new Message Log warnings.

If compile fails, fix compile before visual judgment.

If a presentation element is ugly, correct only that layer; do not reopen the frozen racing foundation.

---

## After Block 3 passes

Continue autonomously with the highest-value remaining Demo 1 work:

### Asset-first art/audio replacement
- replace synthetic fallback sounds with verified free/custom assets where practical
- prioritize engine/load, impacts, countdown/GO/lap/finish, horn and item feedback
- never guess local asset paths

### Environment art replacement
- replace blockout silhouettes gradually with verified free local vegetation/props
- preserve Bangladesh/South-Asian roadside identity
- keep decorative content outside authoritative road collision

### Fresh player-test build
- create a new standalone Shipping package after the presentation gate
- run free-only preflight and package verification
- use `docs/PLAYER_TEST_PLAN.md` with outside players

### Demo 2 only after player-test feedback
Potential themes:
- another route/environment variation
- new race/hazard variation
- stronger rival identity
- deeper accessibility/remapping
- side-grade/cosmetic progression
- multiplayer/social work much later

---

## Known acceptable Demo 1 limitations

Not current blockers:
- traffic can still collide in dense conditions
- physics are arcade/prototype rather than commercial motorcycle simulation
- blockout-quality art remains in places
- one main course/mode is acceptable for Demo 1
- multiplayer is deferred
