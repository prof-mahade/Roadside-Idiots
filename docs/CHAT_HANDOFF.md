# Roadside Idiots — Chat Handoff

Read this first in a new development chat, then inspect `dev/mvp-foundation` and `docs/NEXT_MILESTONE.md` before changing code.

## Project
Roadside Idiots is a Windows PC motorcycle racing game with believable road motion and deliberately funny, petty rider behavior. Long term it is mainly multiplayer; current work is a solo playable prototype.

Tagline: **The road is dangerous. The riders are worse.**

## Branch / local environment
- stable milestone branch: `main`
- active development branch: `dev/mvp-foundation`
- local clone: `C:\GameDev\Roadside-Idiots`
- Unreal Engine 5.8.1
- Visual Studio Community 2026, Game Development with C++
- imported local binary assets are intentionally not stored in Git

## Frozen playable baseline (through VPR-18)
User accepted VPR-18 on 2026-08-15 as **"not bad, playable for now"**. Do not retune the proven core unless a real regression is observed.

Foundation:
- one player + three motorcycle bots
- W accelerate, S brake/reverse, A/D steer
- Q/E slap, F banana peel, G rotten egg, R recovery, Enter restart
- assisted balance/lean and lateral grip
- 12 m oval course with one continuous authoritative collision floor
- invisible road-bump bug fixed and repeatedly re-verified
- real 3-2-1-GO input lock and three-lap race
- checkpoints, time, place and circular top-right minimap
- Condition/damage + visible bandage stages
- LEECH / HOTHEAD / PETTY rival personalities and grudges
- banana pickup/heal/peel loop
- rotten egg pickup/throw/stink/grudge loop
- dog/cow poop hazards
- three civilian traffic vehicles
- bots collect/use the same peel and rotten-egg actions as the player
- anti-bunching AI spacing/braking
- comic impact bursts and crash/dizzy camera response

Imported local presentation assets:
- UE Third Person Manny
- Fab `MotoInteractionAnims`
- `SM_Bike`
- riding/mounted/punch/get-hit/dizzy/interaction animations

Authoritative architecture:
- hidden cube chassis remains authoritative physics
- motorcycle + Manny are presentation-only meshes
- analytic route = 40-point ellipse, radii 9000 cm × 5000 cm
- road width = 1200 cm
- continuous flat collision floor remains authoritative
- segmented road/environment presentation pieces are collision-disabled
- speed-sensitive FOV roughly 92 -> 101 degrees

## Rival AI / optimization
- BOT_01 LEECH: pursuit/grudge focused
- BOT_02 HOTHEAD: aggressive/egg focused
- BOT_03 PETTY: peel oriented
- steering/control loop ~= 20 Hz
- expensive awareness scans ~= 5 Hz and staggered
- item decisions separately throttled
- stuck state is per controller
- AI sensing/stuck logic pauses during countdown
- direct rider blockage causes spacing steer + slowing/braking

## Items / hazards
Banana:
- eight pickups
- heals up to 12 Condition and grants peel
- max 3 peels
- F drops gravity-driven peel
- short owner immunity prevents instant self-hit
- owner can still circle back and slip later
- VPR-18 pickup uses a two-segment banana-like silhouette; dropped peel uses three visual lobes

Rotten egg:
- max 2 per bike
- human/AI share throw path
- SPLAT + wobble + 1 Condition + stink + grudge attribution
- repeated egg hits refresh one stink actor
- VPR-19 pickup slowly rotates for readability

Poop:
- 3 dog piles + 3 cow patties
- dog: short sideways skid/wobble + shorter filth
- cow: speed cut to roughly 42% + longer filth
- one poop mess actor max per bike
- VPR-19 presentation makes dog poop stacked/small and cow poop broad/flat

## Civilian traffic
- yellow SUNDAY DRIVER ~42 km/h
- blue TAXI ~58 km/h with slight wander
- orange DELIVERY VAN ~72 km/h
- overlap-impact architecture avoids hard kinematic deadlocks
- pre-GO racer/traffic contact ignored

## Proven presentation history
VPR-14/14.1:
- minimap, LAP/POS/time, three-lap flow, HUD cleanup, anti-bunching and DIZZY/camera wobble proven

VPR-15:
- comic WHACK/impact presentation proven
- compact HUD/minimap readable
- FILTH status integrated

VPR-16/16.1:
- asphalt/lane/barrier/green-roadside/start-finish presentation works without collision changes
- instancing reduced PIE actor count from ~542 to ~172

VPR-17:
- UE 5.8 Unity Build succeeded
- actor count ~173
- collision-disabled road-join/barrier polish retained
- generated asset-first fallback one-shot audio accepted for prototype use
- UE-owned `SoundWaveProcedural.h` emits a non-fatal C4996 warning

VPR-18 — PLAYABLE BASELINE:
- presentation caches human bike instead of scanning every presentation tick
- lightweight generated player `EnginePulse` varies with speed/throttle
- throttled `TireSkid` cue on hard steering/braking at speed
- banana pickup/drop readability improved
- user accepted overall result as playable for now

VPR-19 — PASSED 2026-08-15:
- one additional collision-disabled roadside-theme root using instanced mesh groups
- South-Asian/Bangladesh-inspired graybox scaffold: stalls/houses, tin-style roofs, shelter, fields/water, vegetation, utility poles/lines
- PIE actor count observed at ~174, confirming theme remained lightweight
- screenshots show theme visible outside the road while three-lap race/minimap/traffic remain stable
- VPR-19 fulfilled its purpose as a scale/density scaffold; do not add more primitive scenery for its own sake

## Audio architecture
`RIAudioEvents` is asset-first:
- checks `/Game/Audio/SFX/SFX_<Event>.SFX_<Event>`
- real imported SFX override generated fallbacks automatically
- generated fallbacks include countdown/GO/lap/finish, impacts, honk, eggs, peel, poop, pickups, engine pulse and tire skid
- distant fallback events receive simple player-distance volume reduction

## CURRENT ACTIVE GATE — VPR-20 Real Art Import
Goal: prove the first real Fab environment-asset replacement pipeline while keeping VPR-18 gameplay and VPR-19 performance architecture frozen.

Current selected first-pass assets are free-at-research-time Fab packs:
1. `tropical Vegetation: Banana Plants` by Project Nature
2. `tropical Vegetation: Ground Plants` by Project Nature
3. `Indian Building Compound Gate - Game Ready Asset Pack (Unreal Engine Only)` by SankoolArts

Why these first:
- all have Unreal Engine formats
- they provide regionally useful vegetation/boundary detail without requiring a paid village purchase
- VPR-20 only needs to prove optional real-art integration, not final-map completeness

Import protocol:
1. user adds the three packs locally through Fab
2. inspect Content Browser and record exact imported folder + candidate Static Mesh names
3. code uses explicit optional asset paths, with primitive fallback if an asset is unavailable
4. imported environment replacements remain NoCollision and should stay instanced where practical
5. actor count should remain approximately near the current ~174 baseline plus transient gameplay actors
6. no imported content may alter the authoritative road/barrier collision or race mechanics

Avoid `Megaplants: Giant Bamboo` for this first gate because its listing currently relies on Experimental Procedural Vegetation / Nanite Foliage features.

Paid architecture remains optional. Regional Indian village-house Fab kits exist, but do not purchase one until the free VPR-20 art pipeline is proven and the user explicitly decides the visual benefit is worth the cost.

## Known limitations still deferred
- bot corner/off-track recovery remains imperfect
- motorcycle physics remain prototype physics
- final sounds/map/traffic/item/environment art not implemented
- sophisticated final traffic physics deferred
- multiplayer networking deferred

## New-chat protocol
1. Read this file.
2. Read `docs/NEXT_MILESTONE.md`.
3. Inspect current `dev/mvp-foundation` head and recent commits.
4. Treat GitHub as more current than old chat text.
5. Continue from the active local gate instead of rebuilding old milestones.
