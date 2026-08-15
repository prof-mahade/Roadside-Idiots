# Next milestone — VPR-14 Race Readability Gate

VPR-13 AI item parity + stink presentation ran successfully in the user's latest screenshot and is good enough to move forward.

## Immediate goal
Turn the prototype from a one-loop mechanics test into a readable race: real laps, real countdown, circular minimap, position/time strip and cleaner AI runtime behavior.

## VPR-14 gate
After pulling/compiling latest `dev/mvp-foundation`, verify:
- HUD shows `BUILD: VPR-14 | MINIMAP + 3 LAPS | AI: OPTIMIZED`
- existing items/hazards/traffic/AI still run

### Start flow
- bikes stay stationary during 3 / 2 / 1
- GO appears and controls release
- player cannot slap/drop/throw before GO
- AI does not recover itself as "stuck" during the intentional countdown freeze
- pre-race traffic contact should not unfairly drain Condition before GO

### Race / laps
- race is 3 laps
- lap 1 completion advances to LAP 2/3 instead of finishing
- lap 2 advances to LAP 3/3
- lap 3 finishes normally
- placement compares completed laps before checkpoint progress
- finish panel displays place and elapsed race time
- Enter restart starts a fresh countdown/race

### Circular minimap
Top-right map should show:
- round course ring
- start/finish tick
- yellow player marker + heading
- three rival markers
- angry rival marker turns red
- small neutral civilian-traffic markers
- markers stay within the map frame while circulating

### HUD
- top-center strip shows `LAP x/3`, `POS x/4`, and running time
- left debug data remains available for prototype diagnostics
- player rotten-egg count comes directly from `ARIBikePawn`, matching shared inventory architecture

### AI optimization regression
- steering remains responsive
- pickup/hazard/traffic avoidance still works
- AI still collects and uses items
- expensive world-awareness scans are throttled/staggered instead of running on every 20 Hz steering tick
- stuck timer is now stored on each AI controller, not in a static map

## Regression checks
- flat road stays flat
- Q/E combat works after GO
- F banana peel works
- G rotten egg works
- AI P#/E# labels still update
- traffic continues circulating
- dog/cow poop + stink still work
- Condition/damage tuning remains stable
- R recovery still works

## If VPR-14 passes
Move into presentation/feel, not another large item architecture change:
1. first free/placeholder audio layer: engine, slap, whack, honk, skid, splat, gross hazard sounds
2. stronger crash/dizzy comedy state using already-imported dizzy/get-hit animations
3. controlled impact/hazard VFX
4. cleaner final-style HUD replacing the diagnostic text gradually
5. environment/map art and final asset replacement later

## Still deferred
- final models/textures/environment
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- multiplayer networking
