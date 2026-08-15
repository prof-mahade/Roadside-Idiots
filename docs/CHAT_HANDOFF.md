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
- ordered checkpoints, place/progress HUD, finish and Enter restart
- R safe recovery
- 12 m oval prototype road, continuous collision floor; invisible road-bump bug fixed
- Q/E slap left/right with impact wobble, SMACK/WHACK feedback and camera kick
- deterministic rival grudge personalities: LEECH, HOTHEAD, PETTY
- Condition/damage governor and visible bandage stages
- banana pickup/heal/peel hazard loop
- rotten egg pickup/throw/stink/grudge loop
- civilian traffic loop

## Important known limitations
- bot corner/off-track recovery is still imperfect and intentionally deferred
- motorcycle physics are prototype physics, not final two-wheel simulation
- final sounds/VFX, final characters, traffic models and item/hazard models are not implemented
- most visuals are engine primitives
- VPR-10.1 stronger rotten-egg stink presentation runs but has not yet been proven by a screenshot with an actively egged rider

## Imported local visuals
Developer machine has:
- UE Third Person Manny (`SKM_Manny_Simple`)
- Fab `MotoInteractionAnims`
- motorcycle skeletal mesh `SM_Bike`
- riding/mounted/punch/get-hit/dizzy/interaction animations

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
- spawn grace: 1.25 s
- banana heals up to 12 Condition
- Condition should change only for readable combat/crash/traffic impacts or healing
- old UE `GEngine not initialized / GetSimplePhysicalMaterial` startup errors were fixed by moving mass overrides into BeginPlay

## Rival personalities — locally verified
- BOT_01 LEECH: long grudge, strong chase, slower attacks
- BOT_02 HOTHEAD: short grudge, fastest chase, aggressive attacks
- BOT_03 PETTY: medium grudge, moderate chase

HUD shows angry rival direction/distance.

## Banana — locally verified
- eight pickups
- pickup heals and grants peel
- carry max 3
- F drops gravity-driven peel
- short self-immunity prevents instant dropper hit
- afterward any bike, including player, can slip
- bot slipping on player peel becomes angry
- user confirmed banana peels work fine

## Combat feel — locally accepted
- slap victim gets roll/yaw wobble
- SMACK over victim, WHACK for player
- brief camera kick
- user feedback: "not bad for now"

## Bandages — locally accepted
- <=75%: arm wrap
- <=50%: head wrap too
- <=25%: calf wrap too
- healing removes stages again
- larger wraps + red accent made them readable from chase camera

## Rotten egg
- carry max 2
- G throws projectile
- hit gives SPLAT, small wobble, 1 Condition damage, stink presentation and NPC grudge
- VPR-10.1 strengthened stink with larger green/brown puffs, brighter light, splatter and STINK label

## Civilian traffic — VPR-11.1 locally accepted
Traffic follows the same analytic oval.
- yellow SUNDAY DRIVER ~42 km/h
- blue TAXI ~58 km/h with slight lane wander
- orange DELIVERY VAN ~72 km/h
- overlap-impact architecture prevents kinematic deadlocks
- touching traffic gives one shove/roll, 6 Condition, HONK/comedy message, 1.25 s pair cooldown
- VPR-11.1 reduced footprint and added compact-car proportions, four wheels and front/rear markers
- latest user screenshot showed the shaped yellow car at sensible scale with overtaking space; treat traffic presentation/movement as passed for prototype purposes

## VPR-12 — dog/cow poop hazards (CURRENT PENDING GATE)
New reusable hazard architecture:
- `ARIPoopHazard`: persistent road hazard with `Dog` / `Cow` type
- `URIPoopWorldSubsystem`: map-level spawner; current prototype map seeds 3 dog piles + 3 cow patties
- `ARIPoopMessEffect`: temporary brown filth presentation following affected bike
- hazards are overlap/query based and persist for multiple riders
- each hazard has per-bike cooldown so sitting on one cannot retrigger continuously
- poop intentionally does **not** change Condition; it is a handling/comedy hazard, not damage

Dog poop behavior:
- small/dark/easy to miss
- sharp sideways skid + roll/yaw wobble
- `SKID! DOG POOP!`
- smaller brown mess for ~4 s

Cow poop behavior:
- much larger road pile
- immediately cuts current horizontal speed to ~42%
- smaller wobble but much heavier slowdown
- `SPLORCH! COW PATTY!`
- larger brown mess for ~6.5 s

HUD marker for current gate:
`BUILD: VPR-12 | HAZARDS: DOG + COW POOP | TRAFFIC: PASSED`

HUD should also show:
`Road hazards: 3 dog poop | 3 cow patties`

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
4. Launch PIE and verify VPR-12 marker plus `3 dog / 3 cow` hazard count.
5. Drive over a small dog pile:
   - obvious sideways skid/wobble
   - DOG POOP message
   - temporary small brown filth
   - Condition should not decrease from poop itself
6. Drive over a large cow patty:
   - speed should drop sharply
   - COW PATTY message
   - larger/longer brown filth
   - Condition should not decrease from poop itself
7. Let an NPC hit a poop hazard if convenient; hazards should work for bots too.
8. Reconfirm traffic, bananas, eggs, flat road and race flow remain intact.
9. If VPR-12 passes, move next to sound/VFX/readability polish or crash/dizzy comedy rather than adding another item immediately.

## New-chat protocol
1. Read this file.
2. Inspect the active branch and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate.
