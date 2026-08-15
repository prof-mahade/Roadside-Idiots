# Next milestone — VPR-12 Dog/Cow Poop Hazard Gate

The solo prototype foundation now has locally accepted road physics, combat impact feel, bandage damage states, banana hazards, rotten eggs and VPR-11.1 shaped civilian traffic.

## Immediate goal
Verify the first map-dependent comedy hazard pair. Dog poop and cow poop must feel mechanically different rather than being banana reskins.

## VPR-12 gate
After pulling and compiling latest `dev/mvp-foundation`, verify:
- HUD shows `BUILD: VPR-12 | HAZARDS: DOG + COW POOP | TRAFFIC: PASSED`
- HUD reports `Road hazards: 3 dog poop | 3 cow patties`
- 3 small dark dog-poop piles and 3 larger cow patties are distributed around the oval

### Dog poop
- small and easier to miss
- touching it produces a quick sideways skid/wobble
- `SKID! DOG POOP!` feedback appears
- smaller brown filth follows the affected bike for about 4 seconds
- poop itself does not reduce Condition

### Cow poop
- substantially larger/clearer pile
- touching it cuts current horizontal speed to roughly 42%
- smaller wobble but strong sticky slowdown
- `SPLORCH! COW PATTY!` feedback appears
- larger brown filth follows the bike for about 6.5 seconds
- poop itself does not reduce Condition

### Persistence / NPCs
- piles remain on the road after being hit
- same bike cannot retrigger a pile continuously because of per-bike cooldown
- NPC bikes can trigger the same hazards

## Regression checks
- road remains flat with no invisible seam bumps
- civilian traffic still circulates normally
- banana pickup/peel loop still works
- rotten egg inventory/throw still works
- Q/E combat and rival grudges still work
- race checkpoints, finish and Enter restart still work

## If VPR-12 passes
Do not add another item immediately. Move into higher-value presentation/feel work:
1. first real impact/honk/slap audio layer using free assets or generated placeholders
2. clearer crash/dizzy comedy state
3. stronger but controlled traffic/hazard VFX
4. later final environment/map art and final asset replacement

## Still deferred
- final bike/car/banana/egg/poop assets
- final character clothing and unique rider models
- sophisticated traffic physics
- perfect bot off-track recovery
- multiplayer networking
