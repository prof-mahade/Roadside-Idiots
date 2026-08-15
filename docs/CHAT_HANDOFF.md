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
- original visible road boxes have collision disabled
- chase camera base: arm 550, height 185, pitch -12.5
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
- peel slip uses comic `SLIP!` / `OWN GOAL!` feedback and shared audio event hook

### Rotten egg
- max 2 per bike
- G throws player egg; AI uses the same shared throw action
- SPLAT + wobble + 1 Condition damage + stink + grudge attribution
- repeated egg hits refresh one stink actor instead of stacking multiple stink actors

### Dog/cow poop
Map seeds 3 dog piles + 3 cow patties.
- dog: quick sideways skid/wobble + shorter filth/stink
- cow: horizontal speed cut to ~42% + longer filth/stink
- at most one poop mess effect per bike; repeated hits refresh/upgrade it
- fumes are three narrow rising wisps rather than large solid green spheres
- rider splats/glow are reduced to keep the motorcycle readable

## Civilian traffic
Traffic follows the analytic oval:
- yellow SUNDAY DRIVER ~42 km/h
- blue TAXI ~58 km/h with slight wander
- orange DELIVERY VAN ~72 km/h
- overlap-impact architecture avoids hard kinematic deadlocks
- pre-GO traffic contact is ignored for racers
- traffic contact fires separate `Honk` + `TrafficHit` audio events

## Passed visual gates
### VPR-14
- circular minimap tracks racers/traffic
- LAP/POS/time strip works
- race advances correctly through multiple laps

### VPR-14.1
- HUD/minimap layout became much cleaner
- stink stopped completely hiding bikes
- pack spacing improved
- DIZZY + short camera wobble retained

### VPR-15
User screenshots on 2026-08-15 proved:
- `VPR-15 | PRESENTATION + AUDIO HOOKS` compiled/runs
- comic WHACK burst and edge-impact treatment are visible
- compact HUD/minimap remain readable
- FILTH status appears correctly inside the left panel
- reduced stink effect remains readable without obscuring the motorcycle
- no visible regression in race presentation

VPR-15 audio architecture:
- `RIAudioEvents` resolves optional `/Game/Audio/SFX/SFX_<Event>` assets
- missing audio stays silent and is cached
- wired events: Countdown, RaceGo, LapComplete, Finish, SlapHit, PeelSlip, EggThrow, EggSplat, EggMiss, DogPoop, CowPoop, Honk, TrafficHit, Crash
- `URIPresentationWorldSubsystem` owns race/countdown/lap/finish/crash presentation cues

## VPR-16 — CURRENT PENDING LOCAL GATE
New `URITrackPresentationSubsystem` is presentation-only. It does not alter the proven road collision or movement code.

Track skin:
- muted green ground overlay
- dark asphalt overlay over checkerboard road
- dashed center guides
- yellow edge lines
- concrete barrier shells + yellow top caps
- checkered start/finish line
- simple start/finish gantry
- sparse roadside trees and colored landmark boards
- all new presentation geometry has collision disabled

Camera feel:
- human chase camera FOV smoothly ranges from roughly 92 degrees at low speed to 101 degrees around 100 km/h
- impact/dizzy camera logic remains separate and unchanged

HUD marker:
`VPR-16 | TRACK SKIN + CAMERA FEEL`

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
4. Launch PIE and verify `VPR-16 | TRACK SKIN + CAMERA FEEL`.
5. Confirm the road is dark asphalt and the outside ground is green rather than checkerboard gray/white.
6. Confirm center/edge lines, barrier caps, checkered start/finish and gantry are visible and aligned.
7. Confirm sparse trees/sign boards remain outside the racing surface.
8. Accelerate from low to high speed and verify FOV widens smoothly rather than snapping.
9. Drive over multiple old road segment boundaries and confirm the invisible-bump bug does not return.
10. Reconfirm minimap, 3 laps, countdown, AI, traffic, slap, peel/egg, poop, Condition and recovery.

If VPR-16 passes, continue into actual prototype sound/engine-loop work and further world-quality replacement without changing core gameplay architecture.

## New-chat protocol
1. Read this file.
2. Inspect active branch and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate.
