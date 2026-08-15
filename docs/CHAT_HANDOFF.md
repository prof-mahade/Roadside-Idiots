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
- `URIPresentationWorldSubsystem` caches human bike instead of scanning every presentation tick
- lightweight generated player `EnginePulse` varies with speed/throttle
- throttled `TireSkid` cue on hard steering/braking at speed
- banana pickup/drop readability improved
- user accepted overall result as playable for now

## Audio architecture
`RIAudioEvents` is asset-first:
- checks `/Game/Audio/SFX/SFX_<Event>.SFX_<Event>`
- real imported SFX override generated fallbacks automatically
- current generated fallbacks include countdown/GO/lap/finish, impacts, honk, eggs, peel, poop, pickups, engine pulse and tire skid
- distant fallback events receive simple player-distance volume reduction

## CURRENT ACTIVE GATE — VPR-19 Roadside Identity
VPR-19 keeps the VPR-18 gameplay baseline frozen and adds presentation only.

New `URIRoadsideThemeSubsystem`:
- one collision-disabled root actor with instanced mesh groups
- reversible South-Asian/Bangladesh-inspired roadside graybox scaffold
- low-rise stall/tea-shop silhouettes
- plaster/brick houses with colored tin-style roofs
- open roadside shelter/bus-stop silhouette
- utility poles/cross-arms/visual overhead lines
- field and water patches beyond barriers
- dirt shoulder/plaza patches and vegetation clusters
- all geometry placed outside the race surface and set to NoCollision

Additional VPR-19 readability:
- dog poop stacked/small vs cow poop broad/flat
- rotten-egg pickup slowly rotates
- no item mechanics/triggers/effects changed

Local VPR-19 gate:
1. pull latest `dev/mvp-foundation` and compile under UE 5.8 Unity Build
2. actor count should rise only slightly above low-170s
3. no theme geometry may intrude onto track or create bumps/collision
4. roadside should feel less empty but not overcrowded
5. dog/cow poop should be visually distinguishable before impact
6. rotten-egg pickup should rotate and collect normally
7. three laps/minimap/AI/items/traffic/Condition/recovery/audio remain stable

## After VPR-19 passes
Do not create more primitive-code scenery for its own sake. Next priority is to choose/confirm a real map art direction and research legally usable environment/SFX assets that fit it, then replace the scaffold gradually while retaining the instanced/collision-safe architecture.

## Known limitations still deferred
- bot corner/off-track recovery remains imperfect
- motorcycle physics remain prototype physics
- final sounds/map/traffic/item/environment art not implemented
- many visuals still use engine primitives
- sophisticated final traffic physics deferred
- multiplayer networking deferred

## New-chat protocol
1. Read this file.
2. Read `docs/NEXT_MILESTONE.md`.
3. Inspect current `dev/mvp-foundation` head and recent commits.
4. Treat GitHub as more current than old chat text.
5. Continue from the active local gate instead of rebuilding old milestones.
