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
- basic bot rivalry/retaliation state after a successful interaction
- overlapping barriers and basic bot stuck recovery
- finish state and instant Enter restart

## Known limitations
- bot corner/off-track recovery is still imperfect and is deferred to a later motorcycle/AI mechanics pass
- current motorcycle physics are prototype physics, not final two-wheel simulation
- final slap/kick timing, sound, comedy VFX and character damage visuals are not implemented

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
- Manny now has a small rider-only seat calibration: slightly rearward and lower relative to the motorcycle origin
- Q/E plays a visible Punch animation
- a successful hit plays a Get_Hits reaction and triggers temporary AI retaliation
- after one-shot interactions, rider returns to the deterministic seated pose

## Readability/layout state
- road width is 1200 cm (12 m)
- barrier height is 120 cm while retaining thick collision
- racer lane offsets use the added road width
- chase camera: arm 550, height 185, pitch -12.5, FOV 95
- HUD build marker for this damage-tuned pass: `BUILD: VPR-04 | ROAD: 12m | DAMAGE: TUNED`
- Enter reloads the current level for an instant clean race restart

## Damage/condition tuning
The first verified VPR-03 playtest showed Condition could collapse from repeated angry-bot attacks and overlapping physics damage. The current branch now uses a damage governor:
- health component ignores additional impact events for 0.65 seconds after a valid hit, preventing one slap plus its physics bump from charging twice
- side interaction damage reduced from 7 to 4
- side impulse reduced from 270 to 220
- Q/E interaction cooldown increased from 0.55 to 0.70 seconds
- AI retaliation has a separate 1.60 second successful-attack cooldown; missed attacks retry after a short delay
- ordinary scrapes no longer count as crash damage; physics collision threshold increased from 18000 to 30000 impulse
- collision damage is capped lower and collision damage cooldown increased to 0.85 seconds
- tip/crash penalty reduced from 4 to 3
- spawn settling still has a 1.25 second damage grace period

The design intent is that Condition should move only for readable combat hits, meaningful crashes, or genuine hard impacts—not constantly while simply racing beside another rider.

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
5. Verify HUD shows `BUILD: VPR-04 | ROAD: 12m | DAMAGE: TUNED`.
6. Test 30–60 seconds:
   - Condition starts at 100/100
   - normal clean riding should leave Condition unchanged
   - light wall/rider rubbing should usually leave Condition unchanged
   - a successful slap should reduce Condition once, not repeatedly from the same contact
   - an angry bot should still retaliate, but should not machine-gun the condition bar
   - rider should sit slightly lower/rearward than VPR-03
   - R recovery and Enter restart still work
7. Send one riding/combat screenshot and report any obvious failure only.

If this gate passes, move to stronger comedy feedback, identifiable rival personalities, crash/rider reaction, then simple traffic/pickups. Do not spend time on final art yet.

## New-chat protocol
1. Read this file.
2. Inspect `dev/mvp-foundation` and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate above.
