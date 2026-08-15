# Roadside Idiots — Chat Handoff

Read this first in a new development chat, then inspect `dev/mvp-foundation` before changing code.

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

## Proven playable foundation
- one player + three motorcycle bots
- W accelerate, S brake/reverse, A/D steer
- assisted balance/lean and lateral grip
- R safe recovery
- 12 m oval prototype road with one continuous collision floor; invisible road-bump bug fixed
- Q/E slap with wobble/reaction/comic impact
- LEECH / HOTHEAD / PETTY rival personalities and grudges
- Condition/damage governor and visible bandage stages
- banana pickup/heal/peel loop
- rotten egg pickup/throw/stink/grudge loop
- civilian traffic loop
- dog/cow poop hazards
- AI item parity: bots collect/use banana peels and rotten eggs
- 3-lap race, countdown, running time, place and circular minimap

## Important known limitations
- bot corner/off-track recovery remains imperfect and is deferred
- motorcycle physics are prototype physics, not final two-wheel simulation
- final sounds, characters, vehicles, map art and item/hazard models are not implemented
- many prototype visuals still use engine primitives
- multiplayer networking is deferred until the solo loop is stronger

## Imported local visuals
Developer machine has UE Third Person Manny, Fab `MotoInteractionAnims`, `SM_Bike`, riding/mounted/punch/get-hit/dizzy/interaction animations.

Presentation architecture:
- hidden cube chassis is authoritative physics
- motorcycle + Manny are presentation-only skeletal meshes
- rider neutral pose = final frame of `AS_Mounted_to_Ride`
- rider has a small lower/rearward seat calibration

## Road / camera
- analytic route: 40-point ellipse, radii 9000 cm × 5000 cm
- road width: 1200 cm
- barrier height: 120 cm
- one continuous flat collision floor is authoritative
- visible road boxes have collision disabled
- chase camera: arm 550, height 185, pitch -12.5, FOV 95
- user confirmed random invisible road bumps are gone

## Condition / damage
- side hit damage: 4
- recipient impact immunity: 0.65 s
- Q/E cooldown: 0.70 s
- hard collision threshold prevents ordinary scrapes counting as damage
- tip/crash penalty: 3
- banana heals up to 12 Condition
- poop intentionally does not directly change Condition
- countdown damage grace prevents unfair pre-GO traffic damage
- old UE `GEngine not initialized / GetSimplePhysicalMaterial` startup errors were fixed by moving mass overrides into BeginPlay

## Rival AI
- BOT_01 LEECH: long grudge, pursuit-focused
- BOT_02 HOTHEAD: fast/aggressive, strongest egg pressure
- BOT_03 PETTY: peel-oriented

Shared item architecture:
- every bike owns peel and rotten-egg inventory
- human and AI call the same `DropBananaPeel()` / `ThrowRottenEggAt()` functions
- bots seek useful nearby pickups
- bots avoid traffic, poop, peels and non-target bikes
- HOTHEAD is deliberately more reckless during grudges

Optimization architecture:
- steering/control loop = 20 Hz
- expensive world-awareness scans ~= 5 Hz, staggered by bot
- item decisions throttled separately
- stuck time is per AI controller
- AI sensing/stuck logic is suspended during countdown
- anti-bunching pass slows/brakes bots when another bike directly blocks their lane

## Items / hazards
### Banana
- eight pickups
- heals and grants peel
- max 3
- F drops gravity-driven peel
- short self-immunity prevents instant self-hit
- peel slip now uses comic `SLIP!` / `OWN GOAL!` feedback and shared audio event hook

### Rotten egg
- max 2 per bike
- G throws player egg; AI uses the same shared throw action
- SPLAT + wobble + 1 Condition damage + stink + grudge attribution
- repeated egg hits now refresh one stink actor instead of stacking multiple stink actors

### Dog/cow poop
Map seeds 3 dog piles + 3 cow patties.
- dog: quick sideways skid/wobble + shorter filth/stink
- cow: horizontal speed cut to ~42% + longer filth/stink
- at most one poop mess effect per bike; repeated hits refresh/upgrade it
- VPR-15 fumes are now three narrow rising wisps rather than large solid green spheres
- rider splats/glow were reduced again to keep the motorcycle readable

## Civilian traffic
Traffic follows the analytic oval:
- yellow SUNDAY DRIVER ~42 km/h
- blue TAXI ~58 km/h with slight wander
- orange DELIVERY VAN ~72 km/h
- overlap-impact architecture avoids hard kinematic deadlocks
- pre-GO traffic contact is ignored for racers
- traffic contact now fires separate `Honk` + `TrafficHit` audio events and no longer emits redundant GEngine screen spam

## VPR-14 — locally passed
User screenshots proved:
- circular minimap tracks racers/traffic
- top strip shows LAP/POS/time
- race advances through multiple laps instead of ending after one circuit

## VPR-14.1 — locally visually passed
Latest user screenshot proved:
- dark-backed HUD/minimap layout is much cleaner
- stink no longer completely hides the bike
- riders are more separated instead of sitting in one obvious pile
- minimap/race loop remains intact

Crash/dizzy pass retained:
- tipping triggers `DIZZY!`
- existing get-hit reaction animation is reused
- human camera gets a short decaying wobble
- auto upright remains 2.4 s
- R/auto recovery clears dizzy state

## VPR-15 — CURRENT PENDING LOCAL GATE
### HUD / VFX cleanup
- build marker: `VPR-15 | PRESENTATION + AUDIO HOOKS`
- player filth/egg stink is shown in the compact left status panel instead of repeated world-space stink labels
- rival labels are closer-range and suppressed if they would overlap the left HUD, top race strip or minimap
- active player comic impacts now draw an 8-ray comic burst plus a brief red edge vignette
- poop fumes are narrower wisps
- rotten-egg stink was reduced from five large puffs to three small wisps; glow is much weaker
- repeated rotten-egg stink refreshes existing effect instead of stacking

### Optional audio foundation
`RIAudioEvents` resolves optional assets from `/Game/Audio/SFX/SFX_<Event>` and stays silent if an asset is missing. Missing assets are cached for the editor run so absent local content does not cause repeated load attempts.

Currently wired events:
- Countdown
- RaceGo
- LapComplete
- Finish
- SlapHit
- PeelSlip
- EggThrow
- EggSplat
- EggMiss
- DogPoop
- CowPoop
- Honk
- TrafficHit
- Crash

See `docs/AUDIO_ASSET_CONVENTION.md` for exact names.

`URIPresentationWorldSubsystem` handles race/countdown/lap/finish/crash presentation cues without changing race mechanics.

## Controls
- W accelerate
- S brake/reverse
- A/D steer
- Q/E slap
- F drop peel
- G throw rotten egg
- R recover
- Enter restart

## Immediate next gate
1. Close Unreal.
2. Pull latest `dev/mvp-foundation`.
3. Compile `RoadsideIdiotsEditor`.
4. Launch PIE and verify `VPR-15 | PRESENTATION + AUDIO HOOKS`.
5. Hit cow/dog poop: the status belongs in the left HUD; the green visual should be much thinner than VPR-14.1.
6. Get egged more than once: there should still be only one compact egg-stink effect, not stacked clouds.
7. Slap/hit traffic/peel/egg/poop and confirm comic feedback still works; audio is expected to remain silent until SFX assets are imported.
8. Deliberately tip the player: DIZZY/camera wobble still works and the new presentation subsystem must not interfere with recovery.
9. Reconfirm minimap, 3 laps, countdown, F/G items, AI item use, traffic, Condition and flat road.
10. Watch rival labels near the minimap/top strip; they should disappear instead of drawing over HUD panels.

After VPR-15 compiles/looks good, next work can add actual sound assets/engine loop and then move into environment/map art replacement without another core gameplay refactor.

## New-chat protocol
1. Read this file.
2. Inspect active branch and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate.
