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
- ordered checkpoints, place/progress HUD and condition value
- manual/automatic recovery
- side interaction on Q/E
- bot retaliation/grudge state after a successful interaction
- deterministic prototype rival personalities
- basic bot stuck recovery
- finish state and instant Enter restart
- continuous flat collision floor for the prototype road

## Known limitations
- bot corner/off-track recovery is still imperfect and is deferred to a later motorcycle/AI mechanics pass
- current motorcycle physics are prototype physics, not final two-wheel simulation
- final slap/kick timing, sound, comedy VFX, rider damage visuals and final character/bike art are not implemented
- current banana visuals are engine-primitive placeholders, not final banana/peel art

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
- health component now also supports positive healing for item pickups

Design intent: Condition should move only for readable combat hits, meaningful crashes, genuine hard impacts, or healing—not constantly while simply racing beside another rider.

## VPR-06 rival personality slice — locally verified
The three prototype bots have deterministic personalities so retaliation is readable and funny before final art:
- `BOT_01 [LEECH]`: 28 s base grudge, strong catch-up, slower attacks; designed to feel like the idiot who refuses to leave you alone
- `BOT_02 [HOTHEAD]`: 10 s grudge, fastest catch-up, most aggressive successful-attack cooldown
- `BOT_03 [PETTY]`: 15 s grudge, moderate catch-up, slower attacks

Repeatedly provoking the same bot extends its current grudge by 3 seconds up to a safe cap rather than silently resetting it.

Combat feedback includes personality labels:
- player hit: `SMACK! BOT_XX [PERSONALITY] IS MAD!`
- bot retaliation: `WHACK! BOT_XX [PERSONALITY] hit YOU!`

While a bot is angry at the player, the HUD shows:
`MAD: BOT_XX [PERSONALITY] | <seconds> | AHEAD/BEHIND/LEFT/RIGHT <distance>m`

## VPR-07 banana + in-world readability slice — pending compile/playtest
New code on `dev/mvp-foundation` adds:
- screen-space identity labels projected above nearby bot riders: `BOT_XX [LEECH/HOTHEAD/PETTY]`
- angry labels append `!! MAD !!` and use stronger warning color
- eight prototype banana pickups are distributed around the oval on alternating lane offsets
- pickups are player-only for this first slice so bots do not steal the test item
- banana pickup heals up to 12 Condition and grants one banana peel
- player can carry up to 3 peels
- F drops a peel about 1.75 m behind the bike
- dropped peel lasts 25 seconds or until triggered
- a rival hitting the peel receives a strong lateral/roll wobble, a small Condition cost and a visible reaction
- if a bot slips on the player's peel, that bot calls `NotifyProvokedBy(player)` and becomes angry at the player
- temporary feedback includes `NOM!`, peel deployment and rival-slip messages
- HUD inventory shows `Banana peels: N / 3`

HUD build marker for this pending slice:
`BUILD: VPR-07 | BANANA: PROTOTYPE | RIVALS: LABELED`

## Recovery
Checkpoint recovery stores a predefined road-center location based on the checkpoint transform rather than the rider's exact wall-hugging crossing location. R should return the bike to a cleaner position after the latest successfully crossed checkpoint.

## Controls
- W: accelerate
- S: brake, then reverse at low speed
- A/D: steer
- Q/E: slap/interact left/right
- F: drop one carried banana peel
- R: recover to latest safe checkpoint position
- Enter: restart the race

## Immediate next gate
1. Close Unreal Editor.
2. Pull latest `dev/mvp-foundation`.
3. Compile `RoadsideIdiotsEditor`.
4. Launch Play-In-Editor.
5. Verify HUD shows `BUILD: VPR-07 | BANANA: PROTOTYPE | RIVALS: LABELED`.
6. Verify nearby bots have readable labels above them.
7. Drive through a glowing banana pickup:
   - Condition should recover by up to 12
   - peel inventory should increase by 1
8. Press F while moving:
   - peel count should decrease
   - a glowing peel placeholder should appear behind the player
9. Let a bot hit the peel:
   - bot should visibly wobble/slip
   - bot should become angry at the player
   - HUD/feedback should identify the angry personality
10. Reconfirm normal road remains flat and Condition remains stable outside meaningful hits.

If this gate passes, proceed to comedy combat/crash feedback: stronger slap impact presentation, a dedicated dizzy/crash rider reaction, then traffic and additional hazards such as rotten egg and map-dependent dog/cow poop.

## New-chat protocol
1. Read this file.
2. Inspect `dev/mvp-foundation` and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate above.
