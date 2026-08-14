# Roadside Idiots — Chat Handoff

Read this first in a new development chat, then inspect the current branch and code before making changes.

## Project
Roadside Idiots is a Windows PC motorcycle racing game with believable road motion and deliberately funny, petty rider behavior. Long term it is primarily multiplayer; development begins with a solo prototype.

Working tagline: **The road is dangerous. The riders are worse.**

## Current branch
`dev/mvp-foundation`

## Local environment
- Unreal Engine 5.8.1
- Visual Studio Community 2026 with Game Development with C++
- local clone: `C:\GameDev\Roadside-Idiots`
- editor target has compiled and Play-In-Editor has run successfully

## Proven gameplay foundation
- runtime oval graybox race route
- one player and three bots
- throttle, brake/reverse, steering, assisted balance/lean and lateral grip
- ordered checkpoints, place/progress HUD and condition value
- manual/automatic recovery
- side interaction on Q/E
- bot retaliation/grudge state after a successful interaction
- basic bot stuck recovery
- finish state and instant Enter restart
- continuous flat collision floor for the prototype road

## Known limitations
- bot corner/off-track recovery is still imperfect and is deferred to a later motorcycle/AI mechanics pass
- current motorcycle physics are prototype physics, not final two-wheel simulation
- final slap/kick timing, sound, comedy VFX, rider damage visuals and final character/bike art are not implemented

## Imported local visual assets
The developer locally imported:
- UE Third Person content with `SKM_Manny_Simple`
- Fab pack `MotoInteractionAnims`
- motorcycle skeletal mesh `SM_Bike`
- animation folders including Riding/Turn_V1, Mounted, Combat/Punch, Get_Hits, Dizzy and Interactions

The binary `.uasset` files are local to the developer machine and are not stored in this Git repository.

## Prototype presentation architecture
- existing hidden cube chassis remains authoritative for physics
- `SM_Bike` and `SKM_Manny_Simple` are presentation-only skeletal meshes discovered through Asset Registry
- presentation meshes use absolute location/rotation/scale so the non-uniform physics chassis cannot crush or stretch them
- bike presentation stays in its clean skeletal reference pose for normal riding
- rider neutral pose is deterministic: final seated frame of `AS_Mounted_to_Ride`; do not auto-pick from Riding/Turn_V1 for neutral driving
- Manny has a small rider-only seat calibration: slightly rearward and lower relative to the motorcycle origin
- Q/E plays a visible Punch animation
- a successful hit plays a Get_Hits reaction and triggers AI retaliation
- after one-shot interactions, rider returns to the deterministic seated pose

## Road collision correction
A verified playtest found that the apparently flat road caused random hops. Root cause: the visible road was 40 overlapping rotated collision boxes; Chaos could catch the chassis on internal seams.

Current architecture:
- one continuous flat collision floor under the entire prototype map is authoritative for ground collision
- visible road segments remain for appearance but have collision disabled
- barriers keep normal collision
- verified locally: road now feels flat and the random invisible-bump hopping disappeared

## Readability/layout state
- road width is 1200 cm (12 m)
- barrier height is 120 cm while retaining thick collision
- racer lane offsets use the added road width
- chase camera: arm 550, height 185, pitch -12.5, FOV 95
- Enter reloads the current level for an instant clean race restart

## Damage/condition tuning
Condition uses a prototype damage governor:
- health component ignores additional impact events for 0.65 seconds after a valid hit, preventing slap + physics bump double-charging
- side interaction damage is 4
- side impulse is 220
- Q/E interaction cooldown is 0.70 seconds
- AI successful retaliation has its own personality-dependent cooldown
- ordinary scrapes should not count as crash damage; physics collision threshold is 30000 impulse
- collision damage is capped and has a separate cooldown
- tip/crash penalty is 3
- spawn settling has a 1.25 second damage grace period

Design intent: Condition should move only for readable combat hits, meaningful crashes, or genuine hard impacts—not constantly while simply racing beside another rider.

## VPR-06 rival personality slice
The three prototype bots now have deterministic personalities so retaliation is readable and funny before final art:
- `BOT_01 [LEECH]`: 28 s base grudge, strong catch-up, slower attacks; designed to feel like the idiot who refuses to leave you alone
- `BOT_02 [HOTHEAD]`: 10 s grudge, fastest catch-up, most aggressive successful-attack cooldown
- `BOT_03 [PETTY]`: 15 s grudge, moderate catch-up, slower attacks

Repeatedly provoking the same bot extends its current grudge by 3 seconds up to a safe cap rather than silently resetting it.

Combat feedback now includes personality labels:
- player hit: `SMACK! BOT_XX [PERSONALITY] IS MAD!`
- bot retaliation: `WHACK! BOT_XX [PERSONALITY] hit YOU!`

HUD build marker for this slice:
`BUILD: VPR-06 | RIVALS: PERSONALITY | ROAD: SEAMLESS`

While a bot is angry at the player, the HUD shows:
`MAD: BOT_XX [PERSONALITY] | <seconds> | AHEAD/BEHIND/LEFT/RIGHT <distance>m`

This is temporary prototype readability so the player can understand which visually identical Manny rider is chasing them.

## Recovery
Checkpoint recovery stores a predefined road-center location based on the checkpoint transform rather than the rider's exact wall-hugging crossing location. R should return the bike to a cleaner position after the latest successfully crossed checkpoint.

## Controls
- W: accelerate
- S: brake, then reverse at low speed
- A/D: steer
- Q/E: slap/interact left/right
- R: recover to latest safe checkpoint position
- Enter: restart the race

## Immediate next gate
1. Close Unreal Editor.
2. Pull latest `dev/mvp-foundation`.
3. Compile `RoadsideIdiotsEditor`.
4. Launch Play-In-Editor.
5. Verify HUD shows `BUILD: VPR-06 | RIVALS: PERSONALITY | ROAD: SEAMLESS`.
6. Test each bot if practical:
   - slap BOT_01 and verify LEECH remains angry much longer than the others
   - slap BOT_02 and verify HOTHEAD retaliates more aggressively while angry
   - slap BOT_03 and verify PETTY sits between those extremes
   - verify the MAD HUD line correctly reports relative direction/distance
   - verify Condition remains stable during clean driving and does not collapse from retaliation
   - verify flat-road hopping remains gone
7. Report only obvious gameplay failures or whether the personality differences are understandable.

If this gate passes, proceed to the next comedy slice: stronger physical/visual slap feedback, clearer rival identification in-world, crash/rider reactions, then the first simple road hazard/pickup system (banana is the preferred first hazard because it simultaneously tests pickup, heal, drop/throw, and opponent slip logic).

## New-chat protocol
1. Read this file.
2. Inspect `dev/mvp-foundation` and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate above.
