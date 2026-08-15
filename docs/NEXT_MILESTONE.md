# Next milestone — VPR-20 Real Art Import Gate

VPR-19 passed visually on the user's machine on 2026-08-15.

## Proven baseline through VPR-19
- VPR-18 mechanics/audio/physics/race stack remains the frozen playable baseline
- UE 5.8 Unity Build succeeds; the engine-owned `SoundWaveProcedural.h` C4996 warning is non-fatal
- three-lap race, minimap, finish flow, Condition, items, traffic and rival AI remain playable
- continuous flat collision road remains authoritative; no invisible road-bump regression
- instanced environment architecture keeps PIE actor count low
- VPR-19 roadside theme raised PIE actor count only from ~173 to ~174
- roadside stalls/houses/poles/fields are visible outside the race surface and do not affect collision
- dog/cow hazard silhouettes and rotating rotten-egg pickup remain presentation-only changes

## VPR-20 goal
Stop adding more primitive-code scenery and prove the first real Fab-asset integration pipeline while preserving the frozen gameplay baseline.

## Selected first-pass Fab targets
Use free assets first so the real-art pipeline can be validated before any paid environment purchase.

1. `tropical Vegetation: Banana Plants` by Project Nature
   - Unreal Engine format
   - free on Fab at time of research
   - 14 banana-plant meshes / growth stages plus ground props
   - intended use: roadside vegetation clusters near stalls, fields and water

2. `tropical Vegetation: Ground Plants` by Project Nature
   - Unreal Engine format
   - free on Fab at time of research
   - 5 plant kinds with multiple growth-stage models
   - intended use: grass/undergrowth breakup around structures and field edges

3. `Indian Building Compound Gate - Game Ready Asset Pack (Unreal Engine Only)` by SankoolArts
   - listed as free on Fab at time of research
   - 10 static meshes, compound walls + metal gate variations
   - intended use: culturally closer roadside boundary/gate details without buying a full village pack yet

Avoid the free `Megaplants: Giant Bamboo` in this first integration because its listing requires Experimental Procedural Vegetation / Nanite Foliage features. We do not need experimental rendering features for this prototype gate.

## Import protocol
The developer imports these assets locally through the Fab integration. Binary `.uasset` content remains local and is intentionally not committed to Git.

After import, before code integration:
1. show/search the imported folders in Content Browser
2. record exact folder names and a few candidate Static Mesh asset names
3. integrate only selected meshes into presentation code via soft/static asset paths
4. keep every environment replacement collision-disabled
5. preserve one-root/instanced presentation architecture where practical
6. keep primitive fallback scenery available if an optional real asset is missing

## VPR-20 local gate
After the three free packs are added locally:
1. exact imported folder/mesh paths must be identified
2. real vegetation/wall assets should replace only selected VPR-19 primitive groups
3. PIE actor count should remain near the current ~174 baseline plus transient gameplay actors
4. no imported asset may create road collision, overlap damage, bumps or off-track blocking
5. road/minimap/three-lap race/traffic/AI/items/Condition/recovery remain unchanged
6. visual direction should feel more like a humid South-Asian roadside and less like an engine-primitives demo

## Paid architecture remains optional
Do not purchase a full village pack yet. Regional Fab packs exist, including Indian village-house kits, but the free-art pipeline should be proven first. If the user later wants a more authentic architecture pass, evaluate those paid packs separately before spending money.

## After VPR-20 passes
1. replace more primitive vegetation and boundary pieces using the proven optional-asset path
2. decide whether paid regional architecture is worth it or whether to build/customize a small reusable house/stall kit
3. replace prototype civilian traffic visuals
4. import real SFX to override generated fallbacks
5. revisit bike/rider animation transitions after the environment identity is materially stronger
6. keep multiplayer networking deferred until solo presentation is visually coherent

## Still deferred
- final environment/models/textures
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- multiplayer networking
