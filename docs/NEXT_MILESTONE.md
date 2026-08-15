# Next milestone — VPR-20 Free Real-Art Integration Gate

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

## VPR-20 status — CODED, LOCAL GATE PENDING
The user imported the two approved free Fab vegetation packs locally:
1. `PN_Banana`
2. `PN_tropicalGroundPlants`

The SankoolArts compound/gate pack was removed and must not be used.

Exact free mesh paths selected from the user's Content Browser screenshots:
- `/Game/PN_Banana/Meshes/plants/banana_01_07.banana_01_07`
- `/Game/PN_Banana/Meshes/plants/banana_02_05.banana_02_05`
- `/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_01_04.tropicalPlant_01_04`
- `/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_05_04.tropicalPlant_05_04`

## VPR-20 implementation
`RIRoadsideThemeSubsystem` now:
- loads those four asset paths optionally at runtime
- preserves the imported meshes' authored materials/textures
- creates NoCollision instanced-mesh groups under the existing roadside-theme root actor
- normalizes instance scale from each mesh's bounds so imported author scale cannot create giant vegetation
- replaces the old primitive ball-tree rows whenever banana assets are available
- places mature/medium banana plants and low/tall tropical ground cover safely outside the 12 m race surface
- retains primitive vegetation fallback if local free assets are missing
- does not alter road/barrier collision, race logic, AI, items, traffic, health or bike physics

Expected actor-count effect: effectively unchanged from ~174 because the imported vegetation is components/instances under the existing theme actor, not separate actors.

## VPR-20 local verification gate
1. close Unreal and pull current `dev/mvp-foundation`
2. compile `RoadsideIdiotsEditor Win64 Development`
3. launch PIE and confirm the real banana/ground-plant materials render correctly
4. verify old spherical tree rows are reduced/replaced around roadside clusters
5. confirm vegetation stays outside the barriers/race surface
6. confirm zero collision, overlap, bumps or off-track blocking from imported art
7. verify actor count stays roughly near the ~174 baseline plus transient gameplay actors
8. drive at least one lap and confirm race/minimap/traffic/AI/items remain unchanged

If VPR-20 passes visually, freeze this free-art pipeline and move directly to VPR-21 instead of adding more random Fab packs.

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
