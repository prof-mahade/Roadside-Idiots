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
- 12 m oval prototype road with one continuous collision floor; invisible road-bump bug fixed
- Q/E slap with impact wobble, SMACK/WHACK feedback and camera kick
- deterministic grudge personalities: LEECH, HOTHEAD, PETTY
- Condition/damage governor and visible bandage stages
- banana pickup/heal/peel hazard loop
- rotten egg pickup/throw/stink/grudge loop
- civilian traffic loop
- dog/cow poop road hazards

## Important known limitations
- bot corner/off-track recovery remains imperfect and is deferred
- motorcycle physics are prototype physics, not final two-wheel simulation
- final sounds/VFX, characters, vehicles and item/hazard models are not implemented
- most prototype visuals use engine primitives
- multiplayer networking is still deferred until the solo gameplay loop is stronger

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
- spawn grace: 1.25 s
- banana heals up to 12 Condition
- poop intentionally does not change Condition directly
- old UE `GEngine not initialized / GetSimplePhysicalMaterial` startup errors were fixed by moving mass overrides into BeginPlay

## Rival personalities
- BOT_01 LEECH: long grudge, strong chase, slower attacks
- BOT_02 HOTHEAD: short grudge, fastest chase, aggressive attacks
- BOT_03 PETTY: medium grudge, more peel-oriented behavior in VPR-13

HUD shows rival personality, direction/distance when angry, and in VPR-13 also projected peel/egg inventory (`P# E#`).

## Banana — locally verified
- eight pickups
- pickup heals and grants peel
- carry max 3
- F drops gravity-driven peel
- short self-immunity prevents instant dropper hit
- afterward any bike can slip
- bot slipping on player peel becomes angry
- user confirmed banana peels work fine

## Combat feel — locally accepted
- slap victim gets roll/yaw wobble
- SMACK over victim, WHACK for player
- brief camera kick
- user feedback: "not bad for now"

## Bandages — locally accepted
- <=75% arm wrap
- <=50% head wrap too
- <=25% calf wrap too
- healing removes stages again
- enlarged wraps + red accents are readable from chase camera

## Rotten egg
- carry max 2
- G throws projectile
- hit gives SPLAT, small wobble, 1 Condition damage, stink presentation and NPC grudge
- projectile already supports any source bike, not only the human player

## Civilian traffic — locally accepted
Traffic follows the same analytic oval:
- yellow SUNDAY DRIVER ~42 km/h
- blue TAXI ~58 km/h with slight lane wander
- orange DELIVERY VAN ~72 km/h
- overlap-impact architecture avoids kinematic deadlocks
- VPR-11.1 compact placeholder car proportions/wheels/lights were accepted from user screenshot

## Dog/cow poop — mechanics locally proven, stink upgrade pending
Current map seeds 3 dog piles + 3 cow patties.

Dog:
- small/dark/easy to miss
- sharp sideways skid + wobble
- temporary brown filth

Cow:
- large obvious patty
- current horizontal speed cut to ~42%
- larger/longer brown filth

User screenshots proved VPR-12 spawns and triggers both types, but requested an actual **stinky effect** instead of only brown blobs.

VPR-13 adds to `ARIPoopMessEffect`:
- four animated dirty-green/brown rising fume blobs
- greenish stink glow
- persistent `DOG STINK!` / `COW STINK!` projected HUD marker while filthy
- cow stink is larger/stronger and lasts longer through the existing mess lifetime

## VPR-13 — AI item parity + awareness (CURRENT PENDING GATE)
Major architecture change: item inventory now belongs to each `ARIBikePawn` rather than keeping rotten eggs in a player-only subsystem.

Shared item actions on every bike:
- `AddBananaPeel`
- `DropBananaPeel`
- `AddRottenEgg`
- `ThrowRottenEggAt`

Human F/G input and AI call the same bike functions.

Pickup parity:
- banana pickups now work for AI too: heal + peel
- rotten egg pickups now work for AI too
- player-only debug messages remain player-only

AI intelligence additions:
- seeks nearby useful banana/egg pickups when not in an active grudge
- simple forward awareness/avoidance for civilian traffic
- attempts to dodge dog/cow poop and dropped banana peels
- gives normal non-target bikes some collision clearance
- HOTHEAD becomes more reckless about hazards during a grudge
- uses rotten eggs at suitable riders ahead
- drops banana peels when a suitable victim is following behind
- item cadence is personality-specific: HOTHEAD throws eggs most aggressively; PETTY drops peels most aggressively; LEECH stays more pursuit-focused
- item victim priority: active grudge target first, otherwise nearby human, then nearest rival

Projected rival labels now include current inventory, e.g.:
`BOT_03 [PETTY] | P1 E0`

VPR-13 HUD marker:
`BUILD: VPR-13 | AI: ITEM PARITY | FILTH: STINKY`

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
4. Launch PIE and verify VPR-13 marker.
5. Hit dog and cow poop and confirm visible rising green/brown stink fumes plus persistent DOG/COW STINK label.
6. Watch projected AI labels for `P# E#`; a bot should be able to collect a banana or egg pickup.
7. Follow a bot that has a peel and give it a chance to drop the peel when you are behind.
8. Get in front of a bot carrying an egg and give it a chance to throw at you; HOTHEAD should be easiest to observe.
9. Check basic intelligence: bots should steer around traffic/hazards more often instead of blindly driving through everything.
10. Reconfirm race flow, traffic, slap combat, player bananas/eggs and Condition remain intact.

If VPR-13 passes, next work should be **audio + crash/dizzy comedy + impact VFX/presentation**, not another item architecture refactor.

## New-chat protocol
1. Read this file.
2. Inspect active branch and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate.
