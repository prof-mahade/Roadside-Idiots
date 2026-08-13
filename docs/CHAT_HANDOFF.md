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

## Current implementation
The repository now contains an Unreal Engine 5.8 C++ project foundation.

Implemented in source:
- `ARIGameMode` starts the prototype
- `ARIDemoWorldBuilder` creates a graybox oval road at runtime from engine primitive meshes
- `ARIBikePawn` is a placeholder physics bike with a placeholder rider shape and chase camera
- `URIBikeMovementComponent` provides throttle, braking, steering, assisted balance and lean
- `ARIAIController` drives the same bike class around route points
- one player and three bot racers are spawned automatically
- `URIParticipantComponent` gives each racer a stable match identity
- `URIHealthComponent` stores a simple condition value
- `URIInteractionComponent` provides left/right side interactions
- `ARICheckpoint` and `ARIRaceManager` provide ordered checkpoints, finish state and basic placement
- `ARIDebugHUD` displays speed, condition, progress, place and controls
- automatic recovery after a low-speed tip plus manual recovery on R
- source control ignores generated Unreal folders and prepares Git LFS patterns for future binary assets

No external art assets are required for this first run.

## Current controls
- W: throttle
- S: brake
- A/D: steer
- Q/E: left/right side interaction
- R: recover upright

## Important architecture rules
1. Gameplay logic and presentation stay separate.
2. Single-player first, but stable state must remain multiplayer-aware.
3. AI driving is separate from future AI personality/strategy.
4. Participant identity must not depend on a specific bike instance.
5. Prefer small reusable components over giant Blueprints.
6. Repository state is the project memory; update this file after milestones.

## Deferred ideas
Do not add these until the first local build runs and the bike/race loop is tested: multiplayer, final motorcycle/rider art, advanced rider animation, traffic, item system, bandage visuals, personality/grudge simulation, map-specific hazards, police, weather, progression, final audio/VFX and polished UI.

## Immediate next gate
The code has not yet been compiled on the development PC because Unreal Engine is not available inside ChatGPT's environment. The next required step is a local Unreal 5.8 compile/open test. Fix compiler/runtime issues first; do not add features before that passes.

## New-chat protocol
1. Read this file.
2. Inspect `dev/mvp-foundation` and recent commits.
3. Treat GitHub as more current than old chat text.
4. Continue from the immediate next gate above.
