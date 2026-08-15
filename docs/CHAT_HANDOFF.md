# Roadside Idiots — Chat Handoff

Read this first in a new development chat, then inspect the current branch and code before making changes.

## Project
Roadside Idiots is a Windows PC motorcycle racing game with believable road motion and deliberately funny, petty rider behavior. Long term it is primarily multiplayer; development begins with a solo prototype.

Working tagline: **The road is dangerous. The riders are worse.**

## Branch strategy
- stable milestone branch: `main`
- active development branch: `dev/mvp-foundation`
- VPR-06 personality/seamless-road foundation was fast-forwarded into `main` after local verification

## Local environment
- Unreal Engine 5.8.1
- Visual Studio Community 2026 with Game Development with C++
- local clone: `C:\GameDev\Roadside-Idiots`
- editor target has compiled and Play-In-Editor has run successfully

## Proven gameplay foundation
- runtime oval graybox race route
- one player and three bots
- throttle, brake/reverse, steering, assisted balance/lean and lateral grip
- ordered checkpoints, place/progress HUD and Condition value
- manual/automatic recovery
- side interaction on Q/E
- bot retaliation/grudge state after a successful interaction
- deterministic rival personalities
- finish state and instant Enter restart
- continuous flat collision floor for the prototype road
- banana pickup/heal/peel hazard loop
- combat impact feedback pass accepted as good enough for now
- visible bandage damage progression accepted as readable in VPR-10 screenshot

## Known limitations
- bot corner/off-track recovery is still imperfect and deferred to a later motorcycle/AI mechanics pass
- current motorcycle physics are prototype physics, not final two-wheel simulation
- final slap/kick sound, particles and final character/bike art are not implemented
- banana, bandage and rotten-egg visuals are engine-primitive placeholders
- rotten-egg readability pass VPR-10.1 still needs local verification

## Imported local visual assets
The developer locally imported:
- UE Third Person content with `SKM_Manny_Simple`
- Fab pack `MotoInteractionAnims`
- motorcycle skeletal mesh `SM_Bike`
- animation folders including Riding/Turn_V1, Mounted, Combat/Punch, Get_Hits, Dizzy and Interactions

The binary `.uasset` files are local to the developer machine and are not stored in this Git repository.

## Prototype presentation architecture
- hidden cube chassis remains authoritative for physics
- `SM_Bike` and `SKM_Manny_Simple` are presentation-only skeletal meshes discovered through Asset Registry
- presentation meshes use absolute location/rotation/scale so the non-uniform physics chassis cannot distort them
- bike presentation stays in its clean skeletal reference pose for normal riding
- rider neutral pose is deterministic: final seated frame of `AS_Mounted_to_Ride`
- Manny has a small rider-only seat calibration: slightly rearward and lower relative to the motorcycle origin
- Q/E plays a visible Punch animation
- successful hit plays a Get_Hits reaction and triggers AI retaliation
- rider returns to deterministic seated pose after one-shot interactions

## Road collision correction — verified
The original segmented road caused random invisible bumps because overlapping box colliders produced internal Chaos edges.

Current architecture:
- one continuous flat collision floor under the entire prototype map is authoritative for ground collision
- visible road segments have collision disabled
- barriers keep collision
- local verification: road now feels flat and random hopping is gone

## Layout/camera state
- road width: 1200 cm (12 m)
- barrier height: 120 cm
- racer lane offsets use the added width
- chase camera: arm 550, height 185, pitch -12.5, FOV 95
- Enter reloads the current level for a clean race restart

## Damage/Condition tuning
- health component ignores additional impact events for 0.65 seconds after a valid hit
- side interaction damage: 4
- Q/E interaction cooldown: 0.70 seconds
- AI retaliation has personality-dependent cooldowns
- ordinary scrapes should not count as crash damage; physics collision threshold is 30000 impulse
- collision damage is capped and separately cooled down
- tip/crash penalty: 3
- spawn settling grace: 1.25 seconds
- health supports positive healing for banana pickups
- physics mass overrides were moved from native constructors to `BeginPlay()` to avoid UE 5.8 `GEngine not initialized / GetSimplePhysicalMaterial` startup errors; latest user log no longer showed those strings

Design intent: Condition should move only for readable combat hits, meaningful crashes, hard impacts, or healing.

## Rival personalities — locally verified
- `BOT_01 [LEECH]`: long grudge, strong catch-up, slower attacks
- `BOT_02 [HOTHEAD]`: short grudge, fastest catch-up, most aggressive attack cadence
- `BOT_03 [PETTY]`: medium grudge, moderate catch-up, slower attacks

Repeated provocation extends the current grudge within a safe cap.

HUD shows angry-rival direction/distance:
`MAD: BOT_XX [PERSONALITY] | <seconds> | AHEAD/BEHIND/LEFT/RIGHT <distance>m`

## Banana loop — locally verified
- eight glowing prototype banana pickups around the oval
- pickup heals up to 12 Condition and grants one peel
- carry up to 3 peels
- F drops a peel behind the rider using deferred spawn so it cannot instantly hit the dropper
- peel has a small gravity-driven physics body and visibly falls to the road
- dropper has only a short self-immunity window; after that anyone, including the player, can slip on it
- slip applies visible lateral/roll wobble, small Condition loss and reaction
- if a bot slips on the player's peel, that bot becomes angry at the player
- user confirmed banana peels are working fine

## VPR-08 combat impact feel — locally accepted
- successful slap adds a stronger roll/yaw wobble instead of only sideways drift
- `SMACK!` appears over a victim
- player receiving a hit gets center-screen `WHACK!`
- player camera gets a short directional kick and smoothly settles
- damage values were not changed in this pass
- user feedback: "not bad for now"; do not keep polishing this slice unless later systems expose a problem

## VPR-09/VPR-10 visible damage progression — locally accepted for readability
Health-driven prototype bandages are attached to Manny bones:
- Condition <= 75%: upper-left-arm bandage
- Condition <= 50%: head bandage also visible
- Condition <= 25%: right-calf bandage also visible
- healing hides bandages again when thresholds are crossed upward
- HUD Condition color and damage text use the same thresholds

VPR-10 enlarged the wraps and added dark-red accent strips. User screenshot at 35/100 Condition showed the arm/head bandage presentation clearly from chase-camera distance, so the readability goal is considered passed for prototype purposes.

## VPR-10 rotten egg prototype — mechanic present, readability improved in VPR-10.1
- `URIRottenEggWorldSubsystem` auto-spawns three ugly green rotten-egg pickups near banana-route anchors
- player can carry up to 2 rotten eggs
- direct prototype input polling: `G` throws a rotten egg; no `DefaultInput.ini` edit
- projectile launches forward with gravity and limited range
- rider hit receives `SPLAT!`, a small wobble, 1 Condition damage and reaction animation
- NPC hit by player's egg becomes angry at the player
- VPR-10 user screenshot confirmed the surrounding gameplay state runs, but the stink state was not visually obvious enough at normal distance

VPR-10.1 strengthens readability without changing mechanics:
- stink lifetime increased to 6 seconds
- five larger green/brown puff meshes surround the affected rider instead of three tiny puffs
- brighter green point light
- yellow-green splatter patch attached with the stink actor
- stink actor owner is set to the affected bike
- HUD projects a persistent `STINK!` label above any bike with an active stink actor

HUD marker for current pending gate:
`BUILD: VPR-10.1 | ROTTEN EGG: READABLE | BANDAGES: PASSED`

## Recovery
Checkpoint recovery stores a predefined road-center location based on checkpoint transform rather than the exact wall-hugging crossing position.

## Controls
- W: accelerate
- S: brake, then reverse at low speed
- A/D: steer
- Q/E: slap left/right
- F: drop one carried banana peel
- G: throw one carried rotten egg
- R: recover to latest safe checkpoint
- Enter: restart race

## Immediate next gate
1. Close Unreal Editor.
2. Pull latest `dev/mvp-foundation`.
3. Compile `RoadsideIdiotsEditor`.
4. Launch PIE and verify HUD shows VPR-10.1.
5. Collect a rotten egg and hit an NPC with G.
6. Verify for roughly six seconds:
   - `SPLAT!` appears on impact
   - larger green/brown stink cloud is obvious from normal chase distance
   - yellow-green splatter patch is visible around the rider/body area
   - `STINK!` label remains above the affected rider
   - victim becomes angry at the source as before
7. Reconfirm bandages, banana loop, road and Condition behavior remain normal.

If VPR-10.1 passes, proceed to the next gameplay-value slice rather than further egg polish: likely traffic or the first map-dependent poop hazard, then later replace placeholder visuals/audio with final assets.

## New-chat protocol
1. Read this file.
2. Inspect `dev/mvp-foundation` and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate above.
