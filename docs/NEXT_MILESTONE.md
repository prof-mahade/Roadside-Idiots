# Next milestone — VPR-21 Custom/Free Roadside-Art Gate

VPR-20.1 passed visually on the user's machine on 2026-08-15.

## Permanent project constraint — FREE ONLY
Roadside Idiots must use only:
- assets/tools/content available to the user for $0 under the applicable license, or
- assets/models/materials/audio we create ourselves.

Do not recommend, plan around, purchase, or retain paid packs as a future dependency. If a suitable free asset does not exist, build a lightweight custom replacement.

## Frozen playable baseline
- VPR-18 gameplay/physics/race/audio baseline remains frozen unless a real regression appears
- VPR-19 roadside-theme foundation remains frozen
- VPR-20/20.1 free vegetation integration is now visually accepted
- three-lap race, minimap, finish flow, Condition, items, hazards, traffic and rival AI remain playable
- continuous flat collision road remains authoritative
- all scenery/presentation pieces remain collision-disabled
- instanced presentation architecture remains mandatory

## VPR-20.1 — PASSED
User screenshots confirmed:
- HUD showed `VPR-20.1 | FREE VEG DENSITY + CLEANUP`
- real banana/tropical vegetation rendered correctly
- old round primitive tree rows were effectively removed from the visible roadside
- denser vegetation clusters looked acceptable for the current demo phase
- PIE actor count remained 174
- race/traffic/rivals continued running normally in the screenshots

Approved local free Fab packs remain:
1. `PN_Banana`
2. `PN_tropicalGroundPlants`

Forbidden / removed:
- SankoolArts compound/gate pack

## VPR-21 — CODED, LOCAL COMPILE/VISUAL GATE PENDING
VPR-21 adds a separate `URIRoadsideArtSubsystem` so visual cleanup can be rolled back independently without disturbing the proven VPR-20 vegetation or gameplay systems.

The new custom/free roadside-art layer:
- adds facade/details over the six existing graybox roadside clusters
- creates three readable roadside archetypes: tea/snack stall, rural house frontage, and open shelter/repair stop
- adds awnings, counters, shelves, benches, veranda slabs, doors, windows, posts, trim, steps, signboards, crates, barrels and short broken fence runs
- uses only Engine basic shapes/materials for custom-made geometry
- optionally reuses already-approved free `PN_Banana` ground props:
  - `/Game/PN_Banana/Meshes/props/groundBananas_01.groundBananas_01`
  - `/Game/PN_Banana/Meshes/props/groundBananas_03.groundBananas_03`
  - `/Game/PN_Banana/Meshes/props/rottenLeaves_01.rottenLeaves_01`
- preserves authored materials on those optional free props
- uses uniquely prefixed Unity-build constants to avoid the previous anonymous-namespace redefinition failure
- disables collision and overlap generation on every new component
- keeps all pieces instanced under one new presentation actor
- does not change road/barrier collision, bike physics, AI, race logic, items, hazards, traffic, health, minimap, recovery or vegetation tuning

Expected PIE actor count: roughly 175 instead of 174 because VPR-21 intentionally adds one visual root actor; transient gameplay actors may add more during a race.

## VPR-21 local verification gate
1. pull current `dev/mvp-foundation`
2. compile `RoadsideIdiotsEditor Win64 Development`
3. launch PIE
4. verify the roadside buildings no longer read as only plain boxes: look for windows/doors/awnings/counters/benches/crates/barrels/fence details
5. verify optional ground-banana/leaf props appear near some clusters if their local mesh paths load
6. verify every new detail stays outside the road/barriers
7. verify there are no new bumps, invisible collisions or stuck points
8. verify actor count is roughly ~175 plus transient gameplay actors
9. drive one full lap and confirm race/minimap/traffic/AI/items remain unchanged

If the art is readable but still visually rough, adjust VPR-21 only. Do not reopen the frozen gameplay stack.

## Demo 1 definition
Demo 1 is a packaged Windows solo build; multiplayer is not required.

Required before calling Demo 1 ready:
1. one visually coherent 3-lap course
2. stable player bike + three rivals
3. stable AI item use, traffic, hazards, recovery and finish flow
4. minimap/HUD/countdown/results readable
5. banana/egg/poop/slap comedy loops working
6. usable engine/skid/impact/item/audio feedback
7. free/custom coherent environment dressing
8. simple title/start/restart/quit flow
9. packaged Windows build launches outside the editor
10. final bug/performance/packaging sweep

## Remaining demo milestones
- VPR-21: custom/free roadside-art cleanup — current local gate
- VPR-22: civilian traffic/item/hazard visual cleanup + rider animation polish
- VPR-23: title/menu/pause/settings/restart/quit + Windows packaging flow
- VPR-24: final demo bug/performance/package audit

## Deferred beyond Demo 1
- multiplayer networking
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- final commercial-quality map/assets/audio
- additional maps/modes
