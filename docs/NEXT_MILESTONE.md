# Next milestone — Demo 1 Polish / Player Experience Gate

## Demo 1 status — FUNCTIONALLY COMPLETE

The standalone Windows demo has already been packaged and played outside Unreal Editor. The accepted functional baseline includes:
- configurable 2–6 AI opponents
- selectable 1–5 laps and 0–6 traffic
- stable countdown / lap / place / finish / restart flow
- motorcycle + Manny presentation in packaged builds
- stable Pure-Pursuit-based racing AI without the old recurring wall oscillation
- prototype traffic passing/recovery at an acceptable level
- slap / banana peel / rotten egg / dog-poop / cow-patty gameplay loops
- minimap and race HUD
- title/setup/pause/settings/restart/quit flow
- free vegetation/environment baseline
- free/custom-content-only packaging policy

Do not reopen foundation systems merely because they could be theoretically more sophisticated.

---

## Product north star

Read `docs/GAME_DESIGN_BIBLE.md` before adding meaningful features.

The permanent fantasy is:

> **You are a competent motorcycle rider trying to win while surrounded by idiots.**

The game is **drive-first, chaos-second**. The player, camera, controls and road-following AI must not feel more foolish than the characters.

`docs/PLAYER_TEST_PLAN.md` defines how future builds should be evaluated with real players.

---

## Permanent project constraint — FREE ONLY

Roadside Idiots may use only:
- content/tools/assets available to the user for $0 under the applicable license, or
- content created by this project.

Do not recommend, plan around, buy or retain paid packs.

The removed SankoolArts / CompoundWall_Kit content must never return. `tools/package_demo1.ps1` recursively blocks it in Content and Source/Config references.

---

## Frozen foundation

Do not retune/rewrite without a reproducible regression:
- physical bike movement/physics baseline
- continuous flat authoritative road collision floor
- `ARIRacingLineFollower` low-level Pure-Pursuit driving stack
- checkpoint/lap/place/finish rules
- core camera baseline
- assisted egg targeting behavior
- basic road dimensions/route geometry

High-level gameplay may be layered around these systems but must not replace their responsibilities.

---

## Current polish changes — pending local compile/package verification

Project version: **0.1.1-demo1-polish1**.

### Player choice / race pacing
- race setup now includes `RACE CHAOS`: CLEAN / BALANCED / MAYHEM
- CLEAN reduces director-created trouble
- BALANCED is the intended default
- MAYHEM raises incident frequency without changing the low-level racing controller
- director-created chaos waits about 6 seconds after GO so the race can establish speed/context first
- simultaneous deliberate troublemakers remain capped

### Input / accessibility comfort
- Xbox-style gamepad gameplay mappings added
- D-pad + A menu navigation added
- Start/Menu pauses
- player-only Steering Feel setting added: CALM / NORMAL / QUICK
- steering feel shapes analog response only; keyboard full steering and AI driving are unchanged

Full in-game remapping is a future accessibility task, not part of this safe polish pass.

### HUD / presentation
- stale VPR/build text removed from player-facing race HUD
- nearby rivals are identified primarily by personality instead of BOT numbers
- finish message is more playful and outcome-specific
- final lap gets a small emphasis
- control hints appear early in the race and then clear the road view; pause menu keeps controls available
- menu copy/spacing is more player-facing and less debug-like

### Distribution
`tools/package_demo1.ps1` now:
- defaults to Shipping
- runs free-only preflight
- builds/cooks/stages Win64
- writes build metadata
- writes a player-facing controls/readme file
- creates a shareable ZIP automatically

---

## Immediate gate — compile and visual verification

Before adding another gameplay layer, sync the latest branch and verify this batch on the user's Windows UE 5.8 machine.

Required checks:
1. C++ build succeeds.
2. QUICK RACE menu has seven rows and Chaos can be changed.
3. Settings has Graphics / VSync / Steering Feel / Back.
4. Keyboard menu still works.
5. If a controller is available, basic gamepad/menu mapping works.
6. CLEAN / BALANCED / MAYHEM visibly change frequency without breaking driving.
7. No stale VPR label appears in the race HUD.
8. Control strip disappears after the opening portion of the race.
9. Finish screen / final-lap text do not overlap the minimap or road view badly.
10. Shipping package succeeds and ZIP is produced.

If a UI spacing problem appears, fix presentation only; do not touch the stable racing controller.

---

## Next autonomous work after this gate

### A. Feedback/audio readability
- audit which actions lack distinct sound/visual feedback
- prioritize engine/load, skid, impact, item throw/hit, countdown/lap/finish
- avoid constant comedy noise that masks useful events
- use free/custom content only

### B. Environment identity
Strengthen recognizable South-Asian/Bangladesh-inspired roadside character with lightweight/free/custom content:
- tea stalls / roadside shops
- utility poles/wires
- small signs and roadside clutter
- tropical vegetation clusters
- small town/village rhythm rather than a generic racing circuit
- future traffic silhouettes inspired by local buses/CNG/auto-rickshaw/vans without using paid content

Humor should come from recognizable situations, not mocking people or poverty.

### C. Traffic / chaos readability
- improve advance readability of traffic and hazards
- preserve committed passing and collision recovery
- refine rival personality expression without increasing constant aggression
- add variety through blocking/traps/opportunism rather than endless slapping

### D. Playtest instrumentation
Add lightweight race telemetry later for:
- collisions by type
- recoveries
- overtakes
- item use/hit rates
- incident density
- finish gap
- restart/quit points

Use `docs/PLAYER_TEST_PLAN.md`; do not optimize metrics without observing player experience.

### E. Demo 2 only after polish gate
Candidate Demo 2 themes:
- another route/environment variation
- stronger rival identity
- new race/hazard variation
- progression through meaningful side-grades/cosmetics rather than grind
- deeper options/accessibility including remapping
- local/online social features much later

---

## Known acceptable Demo 1 limitations

Not current blockers:
- traffic can still collide in dense conditions
- physics are arcade/prototype, not commercial motorcycle simulation
- environment/art/audio are still prototype quality
- one main course/mode is acceptable for Demo 1
- AI tactics can expand later
- multiplayer is deferred

The next goal is **more readable, more distinctive, more replayable**, not “more systems at any cost.”
