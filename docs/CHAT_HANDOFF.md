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
- rider neutral pose is now deterministic: final seated frame of `AS_Mounted_to_Ride`; do not auto-pick from Riding/Turn_V1 for neutral driving
- Q/E plays a visible Punch animation
- a successful hit plays a Get_Hits reaction and triggers temporary AI retaliation
- after one-shot interactions, rider returns to the deterministic seated pose

## Latest readability/layout correction
Pending compile/playtest after latest pull:
- road width increased from 900 cm to 1200 cm
- barrier height reduced from 150 cm to 120 cm while retaining thick collision
- racer lane offsets widened to use the added road space
- chase camera moved farther/higher with wider FOV: arm 550, height 185, pitch -12.5, FOV 95
- spawn settling has a 1.25 second damage grace period so racers should begin at 100 condition
- side interaction reach/radius increased slightly for prototype readability
- successful player hit displays temporary `SMACK! BOT_XX is MAD at you!` feedback
- AI hit on player displays temporary `WHACK! BOT_XX hit YOU!` feedback
- Enter reloads the current level for an instant clean race restart

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
5. Verify:
   - road is visibly wider and walls less tunnel-like
   - camera has better forward road visibility
   - rider uses a believable seated neutral pose instead of Turn_V1
   - condition starts at 100/100
   - Q/E successful hits show clear hit confirmation and visible reaction
   - R recovery still returns to a safe center-road checkpoint location
   - Enter immediately restarts the race
6. Send one screenshot while riding and report any obvious failure only.

If this gate passes, move to the next gameplay slice: stronger slap/hit comedy feedback, identifiable rival personalities, crash/rider reaction, then simple traffic/pickups. Do not spend time on final art yet.

## New-chat protocol
1. Read this file.
2. Inspect `dev/mvp-foundation` and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate above.
