# Next milestone — VPR-24B High-Speed Racing AI Gate

The user rejected the VPR-24A driving quality on 2026-08-15 because rivals still hit the barriers too often and did not look like competent racers. VPR-24B therefore treats racing control as the primary system: opponents must be able to run near the bike's available top speed on straights, plan for bends before reaching them, and dodge traffic/hazards without abandoning the racing line.

## Permanent project constraint — FREE ONLY
Roadside Idiots must use only:
- assets/tools/content available to the user for $0 under the applicable license, or
- assets/models/materials/audio we create ourselves.

Do not recommend, plan around, purchase, or retain paid packs. The removed SankoolArts content must not return; `tools/package_demo1.ps1` blocks it.

## Frozen systems
Do not retune unless a real regression is observed:
- VPR-18 physical bike movement/physics baseline
- continuous flat authoritative road floor collision
- checkpoint/lap/place/finish rules
- traffic movement itself
- player controls and assisted egg behavior
- race setup: 2–6 opponents, 1–5 laps, 0–6 traffic
- title/setup/pause/settings/restart/quit flow

## Why VPR-24A driving failed
The previous AI still used a world-space look-ahead point as the main steering command. Avoidance, pickup seeking and tactical behavior could move that point laterally or blend it toward another actor. At speed the bot therefore over-corrected toward changing targets rather than continuously tracking a stable racing path.

The physical bike supports up to 155 km/h, while VPR-24A personalities were only targeting about 108–116 km/h. Despite being slow, they could still arrive at a bend before the simple angle-based speed limiter had planned enough braking.

The prototype oval was also represented by only 40 straight route/barrier segments. That coarse polygon and the old large barrier padding created unnecessarily abrupt tangent changes and wall-joint geometry for a physics motorcycle.

## VPR-24B controller

### Path tracking owns steering
The racing controller now projects the bike onto the nearest route segment and tracks a smoothed lane reference using three terms:
- heading error toward the local racing-path heading
- Stanley-style lateral/cross-track correction
- preview-curvature feed-forward

Speed increases the look-ahead distance. Avoidance no longer replaces the route target with an arbitrary world position.

### Near-top-speed straights, planned corners
Straight-line personality targets are now roughly 142–151 km/h, below the bike's 155 km/h physical ceiling.

The controller samples several points well ahead, estimates path curvature and converts that curvature into a speed limit using a configurable lateral-acceleration budget. Therefore the AI can stay near full throttle on usable straights but starts slowing before the tight ends of the oval instead of reacting after steering error becomes large.

### Obstacle and hazard avoidance
Traffic, poop, banana peels and other bikes are evaluated as threats using distance plus approximate time-to-collision. The strongest threat requests a bounded lane offset.

The dodge side is barrier-aware:
- prefer moving away from the obstacle
- measure usable road space on both sides
- if the preferred side is too close to the boundary, flip to the safer side
- clamp the requested lane inside the safe corridor
- smooth the lane transition

Braking for obstacles is an emergency fallback. The normal goal is to change lane and preserve speed.

### Wall protection
Three forward wall feelers (center / left / right) provide an emergency steering correction when the bike is actually pointing toward static collision geometry.

The route projection also acts as a corridor guardian. If physics pushes a bike too close to either barrier, tactical/avoidance lane requests are progressively ignored and the controller applies an inward correction until the bike is back on usable asphalt.

### Chaos cannot override a hard corner
Side pressure, blocking and peel-trap lane changes are suppressed during hard turns. Egg shots may still happen because they do not require abandoning the racing line.

### Smoother course reference
The oval dimensions and road width are unchanged, but route/barrier resolution is doubled from 40 to 80 pieces. Checkpoints remain at the same eight fractional positions and pickups remain distributed around the same fractions of the lap. Barrier overhang is reduced to avoid coarse inward wall wedges.

## VPR-24B local verification gate
1. close Unreal and pull current `dev/mvp-foundation`
2. compile `RoadsideIdiotsEditor Win64 Development`
3. HUD must show `VPR-24B | HIGH-SPEED RACING AI`
4. first run Opponents=6, Laps=2, Traffic=0 for a pure path-following test
5. watch at least one full lap: ordinary AI should not touch a barrier repeatedly; a clean lap is the target
6. on straights, fast rivals should build toward roughly 140–150 km/h when unobstructed
7. the tight oval ends may require planned braking; slowing for physical cornering is correct, wall impact is not
8. then run Opponents=6, Traffic=6
9. rivals should normally dodge cars/hazards with a lane change rather than brake to a crawl or aim at a wall
10. traps/traffic must not cause repeated left-right oscillation
11. deliberate chaos must remain short and must not pull bots off-line through hard corners
12. confirm player physics, checkpoint order, lap/finish flow, minimap, pickups, pause/settings and G egg still work

A compile failure, repeated barrier impacts during an unobstructed lap, obvious lane oscillation, or regressions in lap/checkpoint flow block VPR-24B.

## Packaging status
Final Demo 1 packaging stays paused until VPR-24B passes. After that, run the Windows packaged smoke test and final performance/package audit.

## Demo 1 definition
Demo 1 is a packaged Windows solo build; multiplayer is not required.

Required before calling Demo 1 ready:
1. coherent configurable race course
2. 2–6 selectable AI opponents
3. selectable laps and traffic
4. competent high-speed racing AI with occasional distinct chaos
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
- navmesh/Detour/Mass conversion for physics bikes
- final-quality maps/assets/audio
- additional maps/modes
