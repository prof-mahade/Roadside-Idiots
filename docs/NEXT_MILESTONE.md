# Next milestone — VPR-22B / VPR-23A Configurable Chaos Race Gate

VPR-21 and VPR-22A were visually accepted on the user's machine on 2026-08-15.

## Permanent project constraint — FREE ONLY
Roadside Idiots must use only:
- assets/tools/content available to the user for $0 under the applicable license, or
- assets/models/materials/audio we create ourselves.

Do not recommend, plan around, purchase, or retain paid packs as a future dependency. If a suitable free asset does not exist, build a lightweight custom replacement.

## Frozen playable baseline
- VPR-18 bike physics/race/audio baseline remains frozen unless a real regression appears
- continuous flat collision road remains authoritative
- VPR-19 roadside theme, VPR-20 vegetation and VPR-21 roadside-art layers remain accepted
- VPR-22A civilian traffic silhouettes are accepted for the current demo phase
- banana/egg/poop/slap mechanics, recovery, checkpoints and minimap are not being retuned in this gate

## VPR-22A — PASSED
User screenshots confirmed:
- civilian vehicles are visibly more differentiated than the old identical block cars
- presentation-only additions did not change the baseline actor count (175)
- race, rivals, traffic and environment remained playable

## VPR-22B — CODED, LOCAL GATE PENDING
A new `URIRivalChaosSubsystem` sits above the existing AI rather than rewriting stable steering/physics.

It intentionally reuses systems that already work:
- `ARIAIController::NotifyProvokedBy`
- grudge pursuit
- side slap interaction
- rotten-egg targeting
- banana-peel use
- retaliation after being hit

Chaos roles:
- BOT_01 — LEECH: mostly races but periodically stalks a nearby rival
- BOT_02 — HOTHEAD: actively starts fights and strongly prefers reachable AI-vs-AI trouble
- BOT_03 — PETTY: periodically chooses another rider to annoy
- BOT_04 — GREMLIN: disruption is a major objective
- BOT_05 — BRAWLER: prefers close/side-by-side confrontation
- BOT_06 — TRYHARD: intentionally remains race-first so not every opponent behaves the same

The director only assigns nearby sabotage targets and does not alter bike physics. Existing AI hazard avoidance, pickup seeking, crowd braking, recovery and route following remain authoritative between chaos objectives.

## VPR-23A — CODED, LOCAL GATE PENDING
A real C++ pre-race setup screen now controls the race before the world is built.

Backed by `URIRaceSettingsSubsystem`:
- Opponents: hard-clamped to 2–6
- Laps: 1–5
- Traffic: 0–6
- Defaults remain 3 opponents / 3 laps / 3 traffic

The menu uses:
- UP/DOWN — select row
- LEFT/RIGHT — change value
- ENTER on START RACE — build the configured race

Implementation notes:
- `ARIPlayerController` owns setup input
- `ARIRaceSetupHUD` draws the setup menu and falls back to the existing gameplay HUD after launch
- `ARIGameMode` delays `ARIDemoWorldBuilder` until START RACE is confirmed
- race restart still uses the existing level reload; setup values live in the GameInstance subsystem and therefore can remain available when the menu returns
- controller menu key bindings are non-consuming after launch so pawn controls/restart keep working
- traffic spawning waits for the race manager, so changing traffic in the menu actually affects the spawned count
- traffic styling also waits for confirmed race start and cleanly stops when traffic is set to zero

## Scalable start grid
The original fixed 4-rider array was replaced with a three-wide staggered grid:
- player + 2 through player + 6 rivals are supported
- up to 7 total racers use multiple rows
- lane offsets remain comfortably inside the 12 m road
- participant IDs remain PLAYER, BOT_01 ... BOT_06

## Local verification gate
1. pull current `dev/mvp-foundation`
2. compile `RoadsideIdiotsEditor Win64 Development`
3. launch PIE: the race setup menu must appear before the player bike spawns
4. verify Opponents cannot go below 2 or above 6
5. set Opponents = 6, Laps = 1, Traffic = 6 and start once
6. confirm seven total racers spawn without overlap/barrier trapping
7. confirm HUD shows POS x/7 and LAP 1/1
8. confirm roughly six civilian traffic vehicles appear over the lap
9. watch rivals for at least 30–60 seconds and confirm some AI riders chase/slap/use items against other AI riders, not only the player
10. confirm BOT_04/BOT_05/BOT_06 can appear with GREMLIN/BRAWLER/TRYHARD role labels when near the player
11. confirm road collision, recovery, pickups, hazards, finish flow and minimap remain normal
12. press Enter after/restart as before and confirm the level returns to the setup menu

A compile failure or obvious race regression blocks VPR-23A. Minor menu styling does not.

## Demo 1 definition
Demo 1 is a packaged Windows solo build; multiplayer is not required.

Required before calling Demo 1 ready:
1. one visually coherent configurable race course
2. 2–6 selectable AI opponents
3. selectable laps and traffic density
4. AI personalities that include both race-focused and chaos-focused objectives
5. stable items, traffic, hazards, recovery and finish flow
6. minimap/HUD/countdown/results readable
7. simple title/setup/pause/restart/quit flow
8. free/custom coherent environment dressing
9. packaged Windows build launches outside the editor
10. final bug/performance/packaging sweep

## Remaining demo milestones
- VPR-22B / VPR-23A: configurable chaos race + setup menu — current local gate
- VPR-23B: pause/settings/restart/quit/title polish + packaging flow
- VPR-24: final demo bug/performance/package audit

## Deferred beyond Demo 1
- multiplayer networking
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- commercial-quality final map/assets/audio
- additional maps/modes
