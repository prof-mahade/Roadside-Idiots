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
- bot retaliation/grudge state after successful interactions
- deterministic rival personalities
- finish state and instant Enter restart
- continuous flat collision floor for the prototype road
- banana pickup/heal/peel hazard loop
- rotten egg pickup/throw/grudge loop
- combat impact feedback accepted as good enough for now
- visible bandage damage progression accepted as readable
- civilian traffic movement architecture locally proven in VPR-11 screenshot

## Known limitations
- bot corner/off-track recovery is still imperfect and deferred to a later motorcycle/AI mechanics pass
- current motorcycle physics are prototype physics, not final two-wheel simulation
- final slap/kick sound, particles and final character/bike art are not implemented
- banana, bandage, rotten-egg and traffic visuals are engine-primitive placeholders
- VPR-10.1 stronger rotten-egg stink presentation has not yet been visually proven in a screenshot with an actively egged rider
- VPR-11.1 refined traffic shape is pending local compile/visual verification

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
- route is a 40-point ellipse with radii 9000 cm × 5000 cm
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
- physics mass overrides were moved from native constructors to `BeginPlay()` to avoid UE 5.8 startup physical-material errors

Design intent: Condition should move only for readable combat hits, meaningful crashes, hard impacts, traffic impacts, or healing.

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
- F drops a gravity-driven peel behind the rider
- short dropper immunity prevents instant self-hit, after which anyone can slip on it
- slip applies visible lateral/roll wobble, small Condition loss and reaction
- if a bot slips on the player's peel, that bot becomes angry at the player
- user confirmed banana peels are working fine

## Combat impact feel — locally accepted
- successful slap adds stronger roll/yaw wobble
- `SMACK!` appears over a victim
- player receiving a hit gets center-screen `WHACK!`
- player camera gets a short directional kick and smoothly settles
- damage values were not changed in this pass
- user feedback: "not bad for now"

## Visible damage progression — locally accepted
Health-driven prototype bandages are attached to Manny bones:
- Condition <= 75%: upper-left-arm bandage
- Condition <= 50%: head bandage also visible
- Condition <= 25%: right-calf bandage also visible
- healing hides bandages again when thresholds are crossed upward
- HUD Condition color and damage text use the same thresholds

VPR-10 enlarged the wraps and added dark-red accent strips. User screenshots confirmed the wraps are readable from chase-camera distance.

## Rotten egg prototype
- three ugly green rotten-egg pickups
- player can carry up to 2 rotten eggs
- G throws a rotten egg using direct prototype input polling (no DefaultInput.ini edit)
- projectile launches forward with gravity and limited range
- rider hit receives `SPLAT!`, small wobble, 1 Condition damage and reaction animation
- NPC hit by player's egg becomes angry at the player
- VPR-10.1 adds six-second larger green/brown stink puffs, brighter green light, yellow-green splatter patch and persistent `STINK!` HUD marker
- current screenshots prove VPR-10.1 runs, but stronger stink presentation still awaits a screenshot with an actively egged rider

## Civilian traffic — VPR-11 movement locally proven, VPR-11.1 visual pass pending
Traffic uses the same analytic 9000 × 5000 cm oval as the race route.

Movement/behavior:
- three auto-spawned civilian vehicles
- yellow `SUNDAY DRIVER`: ~42 km/h, left-side lane, stable path
- blue `TAXI`: ~58 km/h, right-side lane, slight ~95 cm sinusoidal lane wander
- orange `DELIVERY VAN`: ~72 km/h, near-center lane, stable path
- local VPR-11 screenshot showed traffic present on-route, HUD count 3, and race completion still working
- first collision architecture is overlap-based rather than hard kinematic blocking to avoid traffic/bike deadlocks
- touching traffic gives one shove/roll reaction, 6 Condition impact, `HONK!`, and a comedy message
- each bike/traffic pair has a 1.25 s impact cooldown
- traffic keeps moving instead of becoming stuck against player/NPC physics

VPR-11 screenshot exposed a presentation issue: the simple body/cabin boxes read too much like a giant moving wall. VPR-11.1 keeps the proven movement system but refines visuals/footprint:
- impact envelope reduced to compact-car proportions
- lower/narrower body and cabin
- four visible dark wheels
- front yellow-white lamps and rear red lamps
- vehicle colors/speeds/lane behavior unchanged

Current HUD marker:
`BUILD: VPR-11.1 | TRAFFIC: SHAPED CARS | ITEMS: WORKING`

HUD also shows:
`Traffic: 3 civilian idiots`

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
4. Launch PIE and verify HUD shows VPR-11.1 and `Traffic: 3 civilian idiots`.
5. Drive near the yellow car and verify it now reads approximately like a compact car rather than a giant box/wall.
6. Confirm four wheels/lights are visible and vehicle footprint leaves comfortable overtaking space on the 12 m road.
7. Drive one lap and reconfirm the three traffic actors still follow their lanes smoothly.
8. Touch one traffic vehicle once and reconfirm the existing single shove/Condition/HONK behavior still works.
9. If VPR-11.1 passes, move to the next comedy-value slice rather than further traffic polish: first dog/cow poop road hazard prototype, then audio/final assets later.

## New-chat protocol
1. Read this file.
2. Inspect `dev/mvp-foundation` and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate above.
