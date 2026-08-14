# Next milestone — Banana Hazard & Rival Readability Gate

The VPR-06 foundation is locally verified: seamless flat road, tuned Condition damage, readable retaliation, and deterministic LEECH/HOTHEAD/PETTY personalities. That milestone has been fast-forwarded into `main`. Active work continues on `dev/mvp-foundation`.

## Immediate goal
Validate the first complete funny item loop and make visually identical prototype rivals readable while racing.

## VPR-07 gate
After pulling and compiling the latest `dev/mvp-foundation`, verify:
- HUD shows `BUILD: VPR-07 | BANANA: PROTOTYPE | RIVALS: LABELED`
- nearby bots have projected `BOT_XX [PERSONALITY]` labels above them
- angry rivals append `!! MAD !!`
- glowing banana pickups are visible around the route
- driving through a banana heals up to 12 Condition
- pickup grants one peel and HUD inventory increases
- F drops one carried peel behind the bike
- peel inventory decreases after a successful drop
- another rider touching the peel receives a strong wobble/slip response
- a bot slipping on the player's peel becomes angry at the player
- the personality/grudge HUD remains readable after the peel incident
- seamless road remains flat and stable
- normal clean driving still does not drain Condition

## Design intent
The banana is deliberately the first item because one mechanic exercises multiple core systems at once:
1. world pickup
2. healing/reward
3. small inventory
4. item input
5. spawned road hazard
6. opponent physics reaction
7. visual feedback
8. AI attribution/grudge response

If this works, the same architecture can later support rotten eggs, dog/cow poop, map-specific hazards and other irritating comedy items without rewriting the core item loop.

## If the gate passes
Move into:
1. stronger slap/punch impact presentation
2. dedicated crash/dizzy rider reaction
3. simple traffic actors
4. rotten egg visual impairment prototype
5. map-dependent poop hazards

## Still deferred
- final character/bike art
- final banana/peel models
- final sounds and VFX
- ragdoll-quality crash system
- sophisticated long-term grudge memory
- perfect off-track pathfinding
- multiplayer networking
