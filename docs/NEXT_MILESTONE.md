# Next milestone — VPR-24A Smart Rivals + Assisted Eggs Gate

The user tested the configurable 7-rider build on 2026-08-15 and found the core setup/play loop usable, but correctly rejected the first chaos-AI tuning because too many rivals spent too much of the race fighting instead of racing. VPR-24A is therefore a focused AI/item quality gate before packaging.

## Permanent project constraint — FREE ONLY
Roadside Idiots must use only:
- assets/tools/content available to the user for $0 under the applicable license, or
- assets/models/materials/audio we create ourselves.

Do not recommend, plan around, purchase, or retain paid packs. The removed SankoolArts content must not return; `tools/package_demo1.ps1` blocks it.

## Frozen systems
Do not retune unless a real regression is observed:
- VPR-18 physical bike baseline
- seamless flat authoritative road collision
- checkpoint/lap/place/finish logic
- VPR-19/20/21 environment
- VPR-22A traffic presentation
- race setup: 2–6 opponents, 1–5 laps, 0–6 traffic
- title/setup/pause/settings/restart/quit flow

## Why the old rival behavior failed
The previous chaos layer reused long grudge/chase behavior. With six AI, directive intervals were shorter than some grudge durations, so several riders could remain in direct pursuit at once. While angry, an AI also targeted the rival's current position instead of primarily following the racing line. That caused permanent-looking brawls, bunching and poor driving.

## VPR-24A changes

### Race remains the default state
- ordinary racing-line following is always the base plan
- director-created chaos is intermittent rather than continuous
- 2–4 opponents: at most one deliberate director troublemaker at once
- 5–6 opponents: at most two deliberate director troublemakers at once
- TRYHARD receives no proactive chaos directive and remains race-first
- targets are reserved so deliberate troublemakers do not all dogpile one victim

### Short tactical intents
The director now assigns a temporary intent instead of manufacturing a long grudge:
- `SidePressure` — briefly line up beside a rider and attempt one slap
- `Block` — move into a useful line ahead of a rival without directly aiming for collision
- `PeelTrap` — get ahead/aligned, drop a peel, then disengage
- `EggShot` — stay mostly on the racing line, obtain a firing lane and throw

Typical tactical windows are about 2.8–4.0 seconds, followed by roughly 7–9 seconds of tactical cooldown.

### Fight fatigue / retaliation variety
A hit can still create a short grudge, but it cannot continuously refresh a chase while the victim is already acting or cooling down.

Retaliation is personality-weighted rather than guaranteed:
- HOTHEAD/BRAWLER: often retaliate
- PETTY/GREMLIN: sometimes retaliate
- LEECH: less often
- TRYHARD: usually ignores the fight and keeps racing

### Driving/path-following rewrite
The physical motorcycle model is unchanged. AI control was improved around it:
- project the bike onto the nearest route segment instead of snapping to one of 40 route points
- use speed-dependent look-ahead along the route
- smooth steering, throttle, brake and avoidance inputs
- slow before bends using forward route curvature
- use relative velocity / predicted separation for rider crowd braking
- retain avoidance even when another rider is a tactical target
- clamp final planned target points to a safe road corridor so tactics/avoidance cannot stack into a barrier target
- when almost stationary, reverse-and-steer first; only use recovery if the escape attempt fails

### Rotten egg usability
Player input now explicitly defines `G = ThrowEgg` in `DefaultInput.ini`.

When G is pressed:
- search up to 1000 cm / 10 m
- nearest other race-enabled bike is first priority regardless of human/AI identity
- if a target is found, initial aim leads it slightly and the projectile gets a short homing assist toward its chassis
- if no rider is inside 10 m, the egg still throws forward normally

AI tactical egg shots use the same assisted projectile behavior.

## VPR-24A local verification gate
1. close Unreal and pull current `dev/mvp-foundation`
2. compile `RoadsideIdiotsEditor Win64 Development`
3. HUD must show `VPR-24A | SMART RIVALS + ASSISTED EGGS`
4. run Opponents=6, Laps=2, Traffic=4–6
5. watch at least 60–90 seconds: most riders should be racing most of the time
6. a deliberate fight/trap should be a short event, not a permanent pack brawl
7. observe at least one non-fight tactic over repeated tests if items are available: block, peel trap or egg shot
8. TRYHARD should generally keep racing unless directly provoked
9. watch corners: AI should steer more smoothly and brake earlier instead of waypoint-snapping/weaving
10. if a bot gets boxed/stuck, it should attempt a brief reverse before recovery
11. get an egg, place two rivals within 10 m if possible, press G and verify the nearer one is selected/assisted
12. press G with nobody inside 10 m and verify the egg still fires forward
13. confirm player physics, checkpoints, minimap, traffic, hazards, pause/settings and finish flow did not regress

A compile failure, constant pack fighting, obvious new wall-following, or broken G egg behavior blocks VPR-24A.

## Packaging status
`tools/package_demo1.ps1` remains ready, but final packaging is intentionally paused until VPR-24A passes. Once it passes, run the first Windows package and outside-editor smoke test.

## Demo 1 definition
Demo 1 is a packaged Windows solo build; multiplayer is not required.

Required before calling Demo 1 ready:
1. coherent configurable race course
2. 2–6 selectable AI opponents
3. selectable laps and traffic
4. rivals that primarily race but create occasional distinct chaos
5. stable assisted egg / peel / slap / poop mechanics
6. stable traffic, recovery and finish flow
7. readable HUD/minimap/results
8. title/setup/pause/settings/restart/quit
9. free/custom assets only
10. packaged Windows executable launches outside the editor
11. final package/performance smoke audit

## Deferred beyond Demo 1
- multiplayer networking
- final commercial motorcycle/traffic physics
- sophisticated navmesh/Detour crowd conversion for physics bikes
- final-quality maps/assets/audio
- additional maps/modes
