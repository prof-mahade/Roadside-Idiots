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
- ordered checkpoints, place/progress HUD and Enter restart
- R safe recovery
- 12 m oval prototype road with one continuous collision floor; invisible road-bump bug fixed
- Q/E slap with impact wobble, SMACK/WHACK feedback and camera kick
- deterministic grudge personalities: LEECH, HOTHEAD, PETTY
- Condition/damage governor and visible bandage stages
- banana pickup/heal/peel hazard loop
- rotten egg pickup/throw/stink/grudge loop
- civilian traffic loop
- dog/cow poop road hazards with stink presentation
- AI item parity: bots can collect/use bananas and rotten eggs

## Important known limitations
- bot corner/off-track recovery remains imperfect and is deferred
- motorcycle physics are prototype physics, not final two-wheel simulation
- final sounds/VFX, characters, vehicles and item/hazard models are not implemented
- most prototype visuals use engine primitives
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

## Condition / damage behavior
- side hit damage: 4
- recipient impact immunity: 0.65 s
- Q/E cooldown: 0.70 s
- AI retaliation cooldown is personality-dependent
- hard collision threshold prevents ordinary scrapes counting as damage
- tip/crash penalty: 3
- banana heals up to 12 Condition
- poop intentionally does not change Condition directly
- pre-race damage grace in VPR-14 now covers the countdown so traffic cannot damage a stationary player before GO
- old UE `GEngine not initialized / GetSimplePhysicalMaterial` startup errors were fixed by moving mass overrides into BeginPlay

## Rival personalities / AI
- BOT_01 LEECH: long grudge, strong chase, slower attacks
- BOT_02 HOTHEAD: short grudge, fastest chase, aggressive/egg-oriented behavior
- BOT_03 PETTY: medium grudge, peel-oriented behavior

VPR-13 locally ran successfully and user feedback was that things seemed quite good.

AI parity/awareness:
- all bikes own banana-peel and rotten-egg inventory
- player and bots call the same `DropBananaPeel()` / `ThrowRottenEggAt()` actions
- bots seek useful banana/egg pickups when not actively grudging
- bots attempt to avoid traffic, poop, dropped peels and non-target bikes
- HOTHEAD is deliberately more reckless during grudges
- projected rival labels include `P# E#`

VPR-14 optimization:
- steering still ticks at 20 Hz
- expensive pickup/obstacle scans refresh around 5 Hz and are staggered per bot
- item decisions are throttled separately
- old static controller->stuck-time map was removed; stuck time is now per-controller state
- AI navigation/sensing/stuck logic is suspended during the start countdown

## Items / hazards — locally proven
### Banana
- eight pickups
- pickup heals and grants peel
- carry max 3
- F drops gravity-driven peel
- short self-immunity prevents instant dropper hit
- afterward any bike can slip

### Rotten egg
- carry max 2 per bike
- G throws player egg; AI can throw using same shared bike action
- hit gives SPLAT, small wobble, 1 Condition damage, stink presentation and grudge attribution

### Dog/cow poop
Current map seeds 3 dog piles + 3 cow patties.
- dog = quick sideways skid/wobble + smaller filth/stink
- cow = speed cut to ~42% + larger/longer filth/stink
- rising green/brown fumes and DOG/COW STINK projected label are locally visible
- user screenshot showed VPR-13 cow stink running correctly

## Civilian traffic — locally accepted
Traffic follows the same analytic oval:
- yellow SUNDAY DRIVER ~42 km/h
- blue TAXI ~58 km/h with slight lane wander
- orange DELIVERY VAN ~72 km/h
- overlap-impact architecture avoids kinematic deadlocks
- VPR-11.1 compact placeholder car proportions/wheels/lights were accepted

## VPR-14 — race readability / minimap / laps (CURRENT PENDING LOCAL GATE)
Race architecture:
- race is now 3 laps instead of one checkpoint loop
- `FRIRaceProgress` tracks completed laps, next checkpoint, finish time and last checkpoint time
- place calculation compares finished state -> completed laps -> checkpoint -> checkpoint crossing time
- finish time is stored as elapsed race time

Start flow:
- real 3-second countdown
- player and AI drive controls are held at zero until GO
- slap/item actions are also gated before GO
- HUD shows 3 / 2 / 1 / GO

HUD/minimap:
- compact top-center `LAP / POS / TIME` strip
- circular top-right minimap
- normalized oval route displays as a clean round course
- player marker includes heading
- rival markers use personality/anger colors
- civilian traffic appears as smaller neutral markers
- start/finish tick shown on map
- finish panel shows place + elapsed time + Enter restart
- left debug HUD now reads rotten-egg inventory directly from the player's bike, matching VPR-13 shared-inventory architecture

HUD marker:
`BUILD: VPR-14 | MINIMAP + 3 LAPS | AI: OPTIMIZED`

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
4. Launch PIE and verify the VPR-14 marker.
5. Confirm bikes remain stationary during 3/2/1 and move normally at GO.
6. Check the top-right circular minimap: player + 3 rivals + civilian traffic should move around the ring without leaving the frame.
7. Confirm top-center strip shows LAP 1/3, position and running time.
8. Finish lap 1: race must continue into LAP 2/3 instead of ending.
9. Confirm position still behaves sensibly when riders are on different laps.
10. Finish lap 3: finish panel should show place and elapsed time; Enter restarts with a fresh countdown.
11. Regression: player F/G, AI item use, stink, traffic, Condition and flat road should remain intact.

If VPR-14 passes, next phase is presentation/feel rather than another large architecture refactor: audio layer, crash/dizzy comedy, impact/honk/splat/skid feedback and controlled VFX.

## New-chat protocol
1. Read this file.
2. Inspect active branch and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate.
