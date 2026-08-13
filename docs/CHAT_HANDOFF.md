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
- runtime graybox race route
- one player and three bots
- throttle, brake/reverse, steering, assisted balance/lean and lateral grip
- ordered checkpoints, place/progress HUD and condition value
- manual/automatic recovery
- side interaction on Q/E
- basic bot rivalry/retaliation state after a successful interaction
- taller overlapping barriers and basic bot stuck recovery

## Known limitations
- bot corner recovery is still imperfect and may be redesigned with the later motorcycle/AI mechanics pass
- primitive visuals made interaction feedback too difficult to judge reliably

## Visual readability milestone
The developer locally imported:
- UE Third Person content with `SKM_Manny_Simple`
- Fab pack `MotoInteractionAnims`
- motorcycle skeletal mesh `SM_Bike`
- animation folders including Riding, Mounted, Combat/Punch, Get_Hits, Dizzy and Interactions

The current source now adds a prototype presentation layer that:
- keeps the existing invisible physics chassis authoritative
- discovers `SM_Bike` and `SKM_Manny_Simple` through Unreal Asset Registry
- attaches them as presentation-only skeletal meshes
- hides the old cube/cylinder bike presentation when both assets are available
- selects a riding loop from the imported animation pack
- plays a visible side-action animation when Q/E is used
- plays a visible reaction animation on the other rider when contact succeeds
- returns the rider to the riding loop after the one-shot animation

The imported binary `.uasset` files currently exist only in the local project and have not yet been committed through GitHub.

## Recovery correction
Checkpoint recovery now stores a predefined road-center location based on the checkpoint transform rather than the rider's exact wall-hugging crossing location. R should therefore return the bike to a cleaner position after the next successful checkpoint.

## Controls
- W: accelerate
- S: brake, then reverse at low speed
- A/D: steer
- Q/E: left/right side action
- R: recover to latest safe checkpoint position

## Immediate next gate
1. Close Unreal Editor.
2. Pull the latest `dev/mvp-foundation`.
3. Compile `RoadsideIdiotsEditor`.
4. Launch Play-In-Editor with the locally imported assets present.
5. Verify the real motorcycle and Manny appear.
6. Send screenshots showing alignment/scale before tuning offsets.
7. Test Q/E and R only after the presentation is visually aligned.

Do not move to final art or deeper AI tuning until this visual-readability build is aligned and understandable.

## New-chat protocol
1. Read this file.
2. Inspect `dev/mvp-foundation` and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate above.
