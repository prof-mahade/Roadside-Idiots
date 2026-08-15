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
- Q/E slap with wobble, reaction animation and comic impact text
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
- final sounds/VFX, characters, vehicles and item/hazard models are not implemented
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

Optimization architecture from VPR-14:
- steering/control loop = 20 Hz
- expensive world-awareness scans ~= 5 Hz, staggered by bot
- item decisions throttled separately
- stuck time is per AI controller
- AI sensing/stuck logic is suspended during countdown

VPR-14.1 anti-bunching upgrade:
- non-target bike avoidance is wider/stronger
- cached sense pass calculates crowd speed scaling
- rider directly ahead makes bot slow instead of driving into the pack
- very close blocked lane adds stronger braking
- grudge targets retain a more aggressive minimum following speed

## Items / hazards
### Banana
- eight pickups
- heals and grants peel
- max 3
- F drops gravity-driven peel
- short self-immunity prevents instant self-hit

### Rotten egg
- max 2 per bike
- G throws player egg; AI uses the same shared throw action
- SPLAT + wobble + 1 Condition damage + stink + grudge attribution

### Dog/cow poop
Map seeds 3 dog piles + 3 cow patties.
- dog: quick sideways skid/wobble + shorter filth/stink
- cow: horizontal speed cut to ~42% + longer filth/stink

VPR-14.1 visual cleanup:
- at most one active poop-mess effect per bike
- additional poop hits refresh/upgrade existing mess instead of stacking another set of blobs
- road piles are smaller
- rider splats are smaller
- stink fumes are smaller/tighter and light intensity is reduced
- poop no longer emits redundant `GEngine` screen spam

## Civilian traffic
Traffic follows the analytic oval:
- yellow SUNDAY DRIVER ~42 km/h
- blue TAXI ~58 km/h with slight wander
- orange DELIVERY VAN ~72 km/h
- overlap-impact architecture avoids hard kinematic deadlocks
- pre-GO traffic contact is ignored for racers

## VPR-14 — locally visually passed
The user screenshot proved:
- circular minimap is visible and tracking racers/traffic
- top strip shows LAP/POS/time
- race successfully reached LAP 2/3 instead of finishing after one loop
- VPR-14 race/minimap architecture is good enough to keep

The screenshot also exposed clutter: repeated debug messages, oversized/stacked stink/filth, and a multi-bike bunch around hazards. Those observations triggered VPR-14.1.

## VPR-14.1 — CURRENT PENDING LOCAL GATE
HUD cleanup:
- top-left is now a compact dark-backed gameplay panel
- build marker: `VPR-14.1 | HUD CLEANUP | PACK SPACING`
- top-center race strip has dark backing
- minimap has subtle dark backing and a small `MAP` label
- rival labels only appear when close or actively MAD
- stink labels are smaller/range-limited
- only one concise MAD warning is shown
- controls are fixed to a bottom-left strip
- slap/poop/item-action debug spam was removed where redundant

Crash/dizzy first pass:
- tipping triggers `DIZZY!`
- existing get-hit reaction animation is reused
- human camera gets a short decaying sinusoidal wobble
- crash penalty remains 3 Condition
- auto upright remains 2.4 s
- R/auto recovery clears dizzy state

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
4. Launch PIE and verify `VPR-14.1 | HUD CLEANUP | PACK SPACING`.
5. Confirm the top-left HUD is significantly cleaner and repeated COW PATTY/SMACK messages no longer cover it.
6. Hit dog/cow poop; stink must remain readable but should no longer hide the bike.
7. Hit multiple poop hazards before the effect expires; effect should refresh rather than stack into a wall of spheres.
8. Watch bots converge around riders/hazards; spacing/braking should reduce stationary pile-ups.
9. Deliberately tip the player bike and confirm `DIZZY!` plus a short camera wobble, followed by normal recovery.
10. Reconfirm minimap, three laps, countdown, items, traffic, Condition and flat road.

If VPR-14.1 passes, move to the first audio/presentation package rather than another gameplay architecture refactor.

## New-chat protocol
1. Read this file.
2. Inspect active branch and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate.
