# Next milestone — Readable Combat & Rivalry Gate

The driving prototype and imported motorcycle/Manny presentation are now readable enough to stop judging gameplay through graybox cubes. AI corner/off-track recovery is still imperfect and remains deferred to a later vehicle/AI mechanics pass.

## Immediate goal
Validate that the current visual/readability cleanup makes the core Roadside Idiots interaction understandable while riding.

## Current gate
After pulling and compiling the latest `dev/mvp-foundation`, verify:
- rider sits in a deterministic mounted pose rather than a Turn_V1 animation
- bike/rider proportions remain correct
- widened 1200 cm road gives enough room for overtaking and side interaction
- lower barriers feel less like a tunnel while still containing racers
- chase camera gives better forward road visibility
- race starts at 100 condition instead of losing spawn-settling health
- Q/E produces a visible action
- a successful player hit produces visible reaction plus `SMACK! BOT_XX is MAD at you!`
- an AI counter-hit produces `WHACK! BOT_XX hit YOU!`
- R recovers to the latest safe checkpoint centerline position
- Enter restarts the race immediately

## If the gate passes
Move directly into the first comedy-feedback slice:
1. stronger slap/punch timing and hit weight
2. obvious angry-rival state feedback
3. simple rival identity/personality differences
4. crash/rider reaction pass
5. simple traffic and prototype pickups/road hazards

## Still deferred
- final character/bike art
- final sounds and VFX
- ragdoll-quality crash system
- sophisticated grudge/personality memory
- perfect off-track pathfinding
- multiplayer networking
