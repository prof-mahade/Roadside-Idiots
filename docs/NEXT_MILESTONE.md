# Next milestone — VPR-20 Free Real-Art Import Gate

VPR-19 passed visually on the user's machine on 2026-08-15.

## Permanent project constraint — FREE ONLY
Roadside Idiots must use only:
- free assets that the user can legally acquire for $0 under the applicable license tier, or
- assets/models/materials/audio we create ourselves.

Do not recommend, plan around, or purchase paid packs. Do not keep "paid later" as an option. If a suitable free asset cannot be found, build a lightweight custom replacement instead.

## Proven baseline through VPR-19
- VPR-18 mechanics/audio/physics/race stack remains the frozen playable baseline
- UE 5.8 Unity Build succeeds; the engine-owned `SoundWaveProcedural.h` C4996 warning is non-fatal
- three-lap race, minimap, finish flow, Condition, items, traffic and rival AI remain playable
- continuous flat collision road remains authoritative; no invisible road-bump regression
- instanced environment architecture keeps PIE actor count low
- VPR-19 roadside theme raised PIE actor count only from ~173 to ~174
- roadside stalls/houses/poles/fields are visible outside the race surface and do not affect collision
- dog/cow hazard silhouettes and rotating rotten-egg pickup remain presentation-only changes

## Current VPR-20 goal
Prove the first real free-art integration pipeline while preserving the frozen gameplay baseline.

## Current local asset state
Keep/use only the two free vegetation packs already imported locally:
1. `PN_Banana` — tropical Vegetation: Banana Plants
2. `PN_tropicalGroundPlants` — tropical Vegetation: Ground Plants

The SankoolArts compound/gate pack is being removed and must not be used in the project.

## Import/integration protocol
Binary `.uasset` content remains local and is intentionally not committed to Git.

After the paid/ambiguous pack is deleted:
1. inspect `PN_Banana/Meshes` and `PN_tropicalGroundPlants/Meshes`
2. record exact candidate Static Mesh asset names
3. integrate selected vegetation via optional asset paths
4. every environment replacement remains collision-disabled
5. preserve one-root/instanced presentation architecture where practical
6. retain primitive fallback scenery if an optional free asset is missing
7. build our own simple walls/gates/stalls/houses/signs if no suitable free asset exists

## VPR-20 local gate
1. exact imported mesh paths are identified
2. selected free vegetation replaces some primitive vegetation
3. PIE actor count stays near the current ~174 baseline plus transient gameplay actors
4. no imported asset creates road collision, overlap damage, bumps or off-track blocking
5. road/minimap/three-lap race/traffic/AI/items/Condition/recovery remain unchanged
6. visual direction feels more like a humid South-Asian/Bangladesh roadside and less like an engine-primitives demo

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
- VPR-21: free environment replacement + custom roadside kit cleanup
- VPR-22: civilian traffic/item/hazard visual cleanup + rider animation polish
- VPR-23: menu/title/pause/settings + packaged-build flow
- VPR-24: final demo bug/performance/package audit

If those gates stay small and no major regression appears, Demo 1 is realistically only a few focused development passes away rather than a new long feature phase.

## Deferred beyond Demo 1
- multiplayer networking
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- final commercial-quality map/assets/audio
- additional maps/modes