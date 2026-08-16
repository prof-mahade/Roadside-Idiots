# Next milestone — Demo 1 Polish & Distribution

## Demo 1 status — FUNCTIONALLY COMPLETE
On 2026-08-16 the user explicitly accepted **Demo 1 as functionally complete**.

The standalone Windows build has been packaged and launched outside Unreal Editor with the expected game content present. The accepted Demo 1 baseline includes:
- configurable 2–6 AI opponents
- selectable laps and traffic
- stable race countdown / lap / place / finish / restart flow
- packaged motorcycle + Manny rider presentation
- stable Pure-Pursuit racing AI without the old left/right wall oscillation
- traffic interaction and collision recovery at an acceptable prototype level
- slap / banana peel / rotten egg / poop gameplay loops
- minimap and HUD
- free vegetation/environment presentation
- title/setup/pause/settings/restart/quit flow
- free/custom-content-only packaging rule

The user also verified that six rivals can complete clean races without recurring barrier hits when traffic is disabled, and that dense traffic is playable even though collisions can still happen.

## Permanent project constraint — FREE ONLY
Roadside Idiots must use only:
- assets/tools/content available to the user for $0 under the applicable license, or
- assets/models/materials/audio created by this project.

Do not recommend, plan around, purchase, or retain paid packs.

The removed SankoolArts / CompoundWall_Kit content must never return. `tools/package_demo1.ps1` recursively blocks it in Content and blocks Source/Config references.

## Frozen Demo 1 systems
Do not retune or rewrite these merely for theoretical improvement. Reopen them only for a reproducible regression:
- physical bike movement/physics baseline
- continuous flat authoritative road collision floor
- `ARIRacingLineFollower` Pure-Pursuit driving stack
- checkpoint/lap/place/finish rules
- player controls and assisted egg behavior
- race setup ranges
- title/setup/pause/settings/restart/quit flow
- minimap/HUD functional layout
- free vegetation/environment functional baseline

## Accepted AI architecture
High-level `ARIAIController` owns personality, pace, items, grudges, chaos and tactical intent.

Low-level `ARIRacingLineFollower` owns final driving control:
- speed-scaled Pure Pursuit
- assigned racing lane
- predicted lateral drift recovery
- AI-only lane stabilization
- curvature safety speed veto
- persistent traffic pass-side planning
- post-collision recovery priority

Critical rule: comedy/tactical AI must remain layered around the racing-line follower and must not replace its road-following target again.

## Current packaging baseline
Project version: `0.1.0-demo1-rc2`.

`tools/package_demo1.ps1` defaults to **Shipping** and performs the free-only preflight before BuildCookRun.

Runtime asset cooking is intentionally narrowed to the actual free presentation meshes/materials/textures used by the game. Old UE4 sample Blueprint/map folders bundled with the free vegetation packs are explicitly excluded from cooking.

## Next work — polish, not survival fixes
The next development block should improve perceived quality while preserving the working Demo 1 baseline.

Priority order:
1. remove stale internal VPR/build strings from player-facing HUD and keep version/debug info behind a development-only path
2. improve title/setup/results presentation and spacing
3. improve visual readability of traffic, hazards and item pickups using free/custom content only
4. improve lightweight audio mix/feedback without adding paid assets
5. do a short performance/packaging audit and remove obviously unused cooked content
6. create a clean distributable Shipping archive/ZIP with a concise README, controls and known limitations
7. capture a small set of screenshots/video for Demo 1 presentation

## Known acceptable Demo 1 limitations
These are not blockers for the completed functional Demo 1:
- traffic is prototype/simple and collisions can still occur in dense conditions
- motorcycle/traffic physics are not commercial-simulation quality
- environment/art/audio are intentionally prototype quality
- AI tactics can be expanded later
- only one main race course/mode is required at this stage
- multiplayer is deferred

## Demo 2 direction
Do not begin a major Demo 2 architecture rewrite until the Demo 1 polish/distribution pass is clean.

Likely Demo 2 themes:
- richer rival tactics and personality differentiation
- better overtaking/traffic behavior
- more hazards/items
- stronger environmental identity
- improved traffic models and presentation
- additional race variation/map content
- deeper menu/options progression

Multiplayer remains deferred until the single-player core is substantially more polished.
