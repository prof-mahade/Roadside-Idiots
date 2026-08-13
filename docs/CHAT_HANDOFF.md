# Roadside Idiots — Chat Handoff

Read this first in a new development chat, then inspect the current branch and code before making changes.

## Project
Roadside Idiots is a Windows PC motorcycle racing game with believable road motion and deliberately funny, petty rider behavior. Long term it is primarily multiplayer; development begins with a solo prototype.

Working tagline: **The road is dangerous. The riders are worse.**

## Canonical priority
Get a small playable SOLO MVP running before adding content or polish.

Core loop:
`start race -> ride -> race bots -> side interaction -> wobble/crash -> recover -> checkpoints -> finish`

## Current branch
`dev/mvp-foundation`

## Local development environment
- Unreal Engine 5.8.1 installed
- Visual Studio Community 2026 installed with Game Development with C++ tools
- Local clone: `C:\GameDev\Roadside-Idiots`
- First Unreal editor target compile succeeded on the development PC
- First Play-In-Editor runtime also succeeded

## Current implementation
- `ARIGameMode` starts the prototype
- `ARIDemoWorldBuilder` creates a graybox oval road at runtime from engine primitive meshes
- `ARIBikePawn` is a placeholder physics bike with a placeholder rider shape and chase camera
- `URIBikeMovementComponent` provides throttle, braking, reverse, steering, assisted balance, lean, and lateral grip
- `ARIAIController` drives the same bike class around route points
- one player and three bot racers are spawned automatically
- `URIParticipantComponent` gives each racer a stable match identity
- `URIHealthComponent` stores a simple condition value
- `URIInteractionComponent` provides left/right side interactions
- `ARICheckpoint` and `ARIRaceManager` provide ordered checkpoints, finish state and basic placement
- checkpoints also update a safe recovery location for each racer
- `ARIDebugHUD` displays speed, condition, progress, place and controls
- automatic upright recovery after a low-speed tip
- manual R recovery returns the bike to its latest safe location
- source control ignores generated Unreal folders and prepares Git LFS patterns for future binary assets

No external art assets are required for the current prototype.

## Current controls
- W: accelerate
- S: brake while moving forward; reverse once nearly stopped
- A/D: steer
- Q/E: left/right side interaction
- R: recover to latest safe location

## First playtest findings
The first playable build successfully ran, but the tester found several foundation issues:
- the player could leave the track by getting past the low segmented barriers
- corner steering felt too stiff
- S did not provide reverse movement
- player could become trapped at a corner
- multiple AI riders could pile up and remain trapped at a corner

## Current fix pass awaiting local verification
The current branch contains a follow-up tuning pass that:
- adds reverse behavior to S after braking to low speed
- increases steering response and adds lateral grip
- makes barriers taller, thicker, and overlapping at segment joins
- stores safe recovery transforms at spawn/checkpoints
- makes R return a racer to its latest safe point
- makes AI steer using the nearest route point plus look-ahead
- reduces AI speed for sharper turns
- resets AI to a safe point after remaining nearly stationary for about 1.6 seconds

## Important architecture rules
1. Gameplay logic and presentation stay separate.
2. Single-player first, but stable state must remain multiplayer-aware.
3. AI driving is separate from future AI personality/strategy.
4. Participant identity must not depend on a specific bike instance.
5. Prefer small reusable components over giant Blueprints.
6. Repository state is the project memory; update this file after milestones.

## Deferred ideas
Do not add these until the bike/race loop is stable: multiplayer, final motorcycle/rider art, advanced rider animation, traffic, item system, bandage visuals, personality/grudge simulation, map-specific hazards, police, weather, progression, final audio/VFX and polished UI.

## Immediate next gate
Pull the latest `dev/mvp-foundation`, compile `RoadsideIdiotsEditor`, then run a second Play-In-Editor test focused on:
1. reverse behavior,
2. steering responsiveness,
3. staying inside barriers,
4. manual recovery from a bad position,
5. AI completing corners without permanent pile-ups.

Fix build/runtime or control problems before adding art or new features.

## New-chat protocol
1. Read this file.
2. Inspect `dev/mvp-foundation` and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate above.
