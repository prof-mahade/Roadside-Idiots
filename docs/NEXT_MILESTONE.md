# Next milestone — VPR-20.1 Free Real-Art Refinement Gate

VPR-19 passed visually on the user's machine on 2026-08-15.

## Permanent project constraint — FREE ONLY
Roadside Idiots must use only:
- free assets that the user can legally acquire for $0 under the applicable license tier, or
- assets/models/materials/audio we create ourselves.

Do not recommend, plan around, or purchase paid packs. Do not keep "paid later" as an option. If a suitable free asset cannot be found, build a lightweight custom replacement instead.

## Frozen playable baseline
- VPR-18 mechanics/audio/physics/race stack remains frozen unless a real regression appears
- VPR-19 roadside theme passed at ~174 PIE actors
- three-lap race, minimap, finish flow, Condition, items, traffic and rival AI remain playable
- continuous flat collision road remains authoritative; no invisible road-bump regression
- instanced presentation architecture remains mandatory

## VPR-20 technical asset gate — PASSED VISUALLY
The user's screenshots on 2026-08-15 confirmed that the approved free vegetation assets load and render in PIE while actor count remains ~174.

Approved local free Fab packs:
1. `PN_Banana`
2. `PN_tropicalGroundPlants`

Removed / forbidden:
- SankoolArts compound/gate pack; do not use it.

Exact free mesh paths:
- `/Game/PN_Banana/Meshes/plants/banana_01_07.banana_01_07`
- `/Game/PN_Banana/Meshes/plants/banana_02_05.banana_02_05`
- `/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_01_04.tropicalPlant_01_04`
- `/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_05_04.tropicalPlant_05_04`

## VPR-20.1 implementation — CODED, LOCAL COMPILE/VISUAL GATE PENDING
The first real-art screenshot proved the pipeline but showed vegetation was too sparse and old VPR-16 spherical trees were still visually dominant.

VPR-20.1 therefore:
- roughly doubles roadside vegetation sites around the route
- adds irregular depth, yaw and height variation
- creates occasional two-banana clusters instead of identical isolated trees
- increases low/tall tropical ground-cover density around banana clusters
- keeps every real vegetation instance safely beyond the barrier
- keeps all imported vegetation `NoCollision` with overlap generation disabled
- keeps vegetation instanced under the existing roadside-theme actor, so actor count should remain effectively unchanged
- makes old cube/sphere roadside trees fallback-only when the approved banana pack is present
- preserves primitive fallback scenery when the free pack is missing
- updates the HUD build marker to `VPR-20.1 | FREE VEG DENSITY + CLEANUP`
- does not alter road/barrier collision, bike physics, race logic, AI, items, hazards, traffic, health, minimap or recovery

## VPR-20.1 local verification gate
1. close Unreal and pull current `dev/mvp-foundation`
2. compile `RoadsideIdiotsEditor Win64 Development`
3. launch PIE and verify HUD says `VPR-20.1 | FREE VEG DENSITY + CLEANUP`
4. verify old round lollipop-tree rows are gone when the banana pack is installed
5. verify banana plants now appear in small natural clusters with more ground cover
6. verify plants do not intrude into the road or barrier geometry
7. verify zero collision, overlap, bumps or off-track blocking from imported art
8. verify PIE actor count remains roughly ~174 plus transient gameplay actors
9. drive at least one lap and confirm race/minimap/traffic/AI/items remain unchanged

If this gate passes, VPR-20 is frozen and work moves directly to VPR-21 rather than adding more random asset packs.

## Demo 1 definition
Demo 1 is a packaged Windows solo build and does NOT require multiplayer.

Required before calling Demo 1 ready:
1. one visually coherent 3-lap course
2. stable player bike + three rivals
3. stable AI item use, traffic, hazards, recovery and finish flow
4. minimap/HUD/countdown/results readable
5. banana/egg/poop/slap comedy loops working
6. usable engine/skid/impact/item/audio feedback
7. free real vegetation/environment pass or custom-made equivalents
8. simple title/start/restart/quit flow
9. packaged Windows build launches outside the editor
10. final bug, performance and packaging sweep

## Remaining demo milestones after VPR-20
- VPR-21: custom/free roadside kit cleanup (houses/stalls/walls/signs/props made by us where necessary)
- VPR-22: civilian traffic/item/hazard visual cleanup + rider animation polish
- VPR-23: title/menu/pause/settings/restart/quit + Windows packaging flow
- VPR-24: final demo bug/performance/package audit

If those gates stay small and no major regression appears, Demo 1 remains only a few focused development passes away.

## Deferred beyond Demo 1
- multiplayer networking
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- final commercial-quality map/assets/audio
- additional maps/modes
