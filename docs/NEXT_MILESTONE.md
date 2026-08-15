# Next milestone — VPR-14.1 Presentation Cleanup Gate

VPR-14 is visually proven in the user's screenshot: the circular minimap is active, LAP 2/3 continues correctly, position/time update, and the race did not end after lap 1.

The same screenshot exposed the next cleanup targets: upper-left debug-message clutter, oversized/stacked filth effects, and riders bunching when several bikes meet around hazards.

## Immediate goal
Clean the readable-race layer before adding audio: keep the successful VPR-14 race/minimap architecture, reduce prototype visual noise, improve pack spacing, and add a first crash/dizzy comedy beat.

## VPR-14.1 changes

### Filth / stink cleanup
- only one `ARIPoopMessEffect` may remain active per bike
- crossing another pile refreshes/upgrades the existing mess instead of stacking another full set of splats/fumes
- cow and dog road-pile placeholder meshes are smaller
- rider-attached brown splats are smaller
- green/brown stink fumes are substantially smaller and tighter around the rider
- stink point light is reduced
- poop/combat no longer create redundant `GEngine` screen-message spam over the HUD

### AI anti-bunching
The existing ~5 Hz cached sense pass now also calculates pack spacing:
- stronger side avoidance for non-target bikes
- nearby rider directly ahead reduces desired speed
- very close blocked lane triggers stronger braking
- active grudge target still allows a more aggressive minimum following speed
- steering remains on the existing 20 Hz control loop

Goal: fewer multi-bike deadlocks without turning rivals into passive traffic-following bots.

### Crash / dizzy comedy
- first transition into a tipped state triggers `DIZZY!`
- existing get-hit rider reaction animation is reused for the first pass
- human chase camera gets a short decaying sinusoidal dizzy wobble
- existing 3 Condition crash penalty remains unchanged
- auto-upright timing remains 2.4 s
- R/auto recovery clears dizzy state

### HUD cleanup
- build marker becomes `VPR-14.1 | HUD CLEANUP | PACK SPACING`
- compact dark-backed top-left gameplay panel replaces the long diagnostic list
- top-center LAP/POS/TIME strip gets its own dark backing
- minimap gets a subtle dark backing and moves slightly tighter into the corner
- duplicate LAP label over the map is replaced by a small `MAP` label
- rival world labels are range-limited unless the rival is actively MAD
- stink labels are smaller and range-limited
- only one concise MAD warning is shown in the left HUD
- controls move to a fixed bottom-left strip instead of extending the debug list

## VPR-14.1 local gate
After pulling/compiling latest `dev/mvp-foundation`:
1. confirm the build marker says `VPR-14.1 | HUD CLEANUP | PACK SPACING`
2. run through dog/cow poop and confirm the rider is still visibly filthy/stinky but the effect no longer hides the motorcycle
3. hit multiple poop hazards before the previous mess expires; effect should refresh rather than stack into many spheres/blobs
4. watch several bots converge; they should brake/space themselves more often instead of forming a stationary pile
5. deliberately tip/crash the player bike and confirm `DIZZY!` + brief camera wobble, then normal recovery
6. confirm top-left HUD is much cleaner and no repeated COW PATTY / SMACK screen messages cover it
7. reconfirm minimap, 3 laps, countdown, items, traffic, Condition and flat road

## If VPR-14.1 passes
Proceed to the first audio/presentation package:
1. establish an audio event layer for engine/slap/impact/honk/skid/splat/gross reactions
2. use free/imported placeholder sounds where legally usable and locally available
3. add controlled impact/hazard VFX that do not obscure racing visibility
4. continue replacing prototype debug presentation with final-style HUD elements

## Still deferred
- final environment/models/textures
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- multiplayer networking
