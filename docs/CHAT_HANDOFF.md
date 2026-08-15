# Roadside Idiots — Chat Handoff

Read this first in a new development chat, then inspect `dev/mvp-foundation` and `docs/NEXT_MILESTONE.md` before changing code.

## Project
Roadside Idiots is a Windows PC motorcycle racing game with believable road motion and deliberately funny, petty rider behavior. Long term it is mainly multiplayer; current work is a solo playable prototype.

Tagline: **The road is dangerous. The riders are worse.**

## PERMANENT USER CONSTRAINT — FREE ONLY
The user has explicitly required that the entire project use only:
- assets/tools/content available to them for $0 under the applicable license, or
- assets/models/materials/audio we create ourselves.

Do not recommend or plan around paid packs, subscriptions, or "buy later" content. If a suitable free asset cannot be found, make a lightweight custom replacement.

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
- free `PN_Banana` vegetation pack
- free `PN_tropicalGroundPlants` vegetation pack

Removed / do not use:
- SankoolArts compound/gate pack; user removed it after license/payment concern

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
- dog silhouette stacked/small; cow broad/flat

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
- generated asset-first fallback one-shot audio accepted for prototype use
- UE-owned `SoundWaveProcedural.h` emits a non-fatal C4996 warning

VPR-18 — PLAYABLE BASELINE:
- cached human-bike presentation reference
- lightweight generated player EnginePulse varies with speed/throttle
- throttled TireSkid cue
- banana pickup/drop readability improved
- user accepted overall result as playable

VPR-19 — PASSED:
- one additional collision-disabled roadside-theme root with instanced mesh groups
- Bangladesh/South-Asian-inspired graybox scaffold: stalls/houses, tin-style roofs, shelter, fields/water, vegetation, utility poles/lines
- PIE actor count observed ~174
- theme stayed outside road; three-lap race/minimap/traffic stable
- do not add more primitive scenery for its own sake

## Audio architecture
`RIAudioEvents` is asset-first:
- checks `/Game/Audio/SFX/SFX_<Event>.SFX_<Event>`
- real imported free SFX can override generated fallbacks automatically
- generated fallbacks include countdown/GO/lap/finish, impacts, honk, eggs, peel, poop, pickups, engine pulse and tire skid

## CURRENT ACTIVE GATE — VPR-20 Free Real-Art Integration
Status: CODED, pending local compile + visual verification.

Approved local free mesh paths selected from the user's Content Browser screenshots:
- `/Game/PN_Banana/Meshes/plants/banana_01_07.banana_01_07`
- `/Game/PN_Banana/Meshes/plants/banana_02_05.banana_02_05`
- `/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_01_04.tropicalPlant_01_04`
- `/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_05_04.tropicalPlant_05_04`

`RIRoadsideThemeSubsystem` VPR-20 behavior:
- optionally loads the four free meshes above
- keeps their authored materials/textures
- creates NoCollision instanced components under the existing one roadside-theme root actor
- normalizes instance scale using each mesh's bounds
- replaces old primitive ball-tree rows when banana meshes are available
- adds banana/ground-plant clusters well outside the 12 m race surface
- retains primitive fallbacks if local assets are missing
- does not modify road collision, bike physics, race, AI, items, traffic or health
- expected actor count remains near ~174 because this adds components/instances, not new actors

Next local gate:
1. pull current branch
2. compile
3. verify real materials/scale/placement in PIE
4. drive one lap; confirm no bumps/collision/overlap regression
5. confirm actor count remains near baseline
6. send screenshots only if visual scale/density needs judgment

## Demo 1 definition
Demo 1 is a packaged Windows SOLO build. Multiplayer is not required.

Required:
1. one coherent 3-lap course
2. stable player + 3 rivals
3. AI/items/hazards/traffic/recovery/finish stable
4. minimap/HUD/countdown/results readable
5. comedy loops working
6. usable audio feedback
7. free real environment dressing or custom equivalents
8. simple title/start/restart/quit flow
9. packaged Windows build launches outside editor
10. final bug/performance/package sweep

Planned remaining gates:
- VPR-20: free vegetation integration — current local gate
- VPR-21: custom/free roadside-art cleanup
- VPR-22: traffic/item/hazard visual cleanup + rider animation polish
- VPR-23: menu/title/pause/settings + packaging flow
- VPR-24: final demo audit/package

If no major regression appears, Demo 1 is only a few focused milestone passes away.

## Known limitations deferred beyond Demo 1
- multiplayer networking
- perfect off-track recovery
- sophisticated final motorcycle/traffic physics
- final commercial-quality map/assets/audio
- additional maps/modes

## New-chat protocol
1. Read this file.
2. Read `docs/NEXT_MILESTONE.md`.
3. Inspect current `dev/mvp-foundation` head and recent commits.
4. Treat GitHub as more current than old chat text.
5. Continue from the active local gate instead of rebuilding old milestones.
