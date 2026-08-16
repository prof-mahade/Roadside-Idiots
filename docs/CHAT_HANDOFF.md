# Roadside Idiots — Chat Handoff

Read this first in a new development chat, then read:
1. `docs/GAME_DESIGN_BIBLE.md`
2. `docs/NEXT_MILESTONE.md`
3. `docs/PLAYER_TEST_PLAN.md`
4. inspect current `dev/mvp-foundation` head before changing code.

## Project
Roadside Idiots is a Windows PC arcade motorcycle racing/combat game with believable road motion and deliberately funny, petty rider behavior.

Tagline: **The road is dangerous. The riders are worse.**

Permanent core fantasy:

> **You are a competent motorcycle rider trying to win while surrounded by idiots.**

The game is **drive-first, chaos-second**. Chaos should create readable decisions/stories around skilled riding; the controls, camera, road-following AI and UI should not feel like the real idiots.

## CURRENT STATUS — DEMO 1 FUNCTIONALLY COMPLETE / POLISH CONTINUING
On 2026-08-16 the user explicitly accepted Demo 1 as functionally complete.

A standalone packaged Windows build has launched outside Unreal Editor with the expected motorcycle/rider presentation, track/environment, HUD/minimap, race flow and traffic present.

The user subsequently accepted the `05c2604` AI/racecraft state as a good improvement and accepted the first player engine/skid feedback slice. Those systems are now treated as stable baselines.

The latest autonomous batch adds collision-free roadside identity geometry and passive playtest telemetry. That latest batch is **pending local UE 5.8 compile and visual/gameplay verification**.

Do not reopen solved foundation problems merely for theoretical improvement.

## PERMANENT USER CONSTRAINT — FREE ONLY
The entire project must use only:
- assets/tools/content available to the user for $0 under the applicable license, or
- assets/models/materials/audio created by the project.

Do not recommend or plan around paid packs, subscriptions or “buy later” content. If suitable free content cannot be used, make a lightweight custom replacement.

Removed and forbidden:
- SankoolArts compound/gate pack
- any `CompoundWall_Kit` copy/reference

`tools/package_demo1.ps1` enforces this before packaging.

## Branch / local environment
- stable branch: `main`
- active branch: `dev/mvp-foundation`
- local clone: `C:\GameDev\Roadside-Idiots`
- Unreal Engine 5.8.1
- imported binary presentation assets are intentionally local and may appear as untracked `Content/`

## Working style with the user
The productive workflow is intentionally incremental:
1. inspect the current repository state
2. make one coherent technical slice
3. give exact PowerShell sync/build/launch commands when user execution is required
4. ask the user only for observations that genuinely require human visual/audio/gameplay judgment
5. freeze accepted systems instead of repeatedly retuning them

When the user says to work autonomously, continue through safe additive work and repository bookkeeping until local compile or human play-feel verification is actually needed.

## Accepted Demo 1 gameplay baseline
- W/S throttle/brake-reverse, A/D steer
- Q/E slap, F banana peel, G rotten egg, R recovery
- P/Esc pause, Enter restart/confirm
- Xbox-style gamepad gameplay/menu mapping
- configurable 2–6 opponents, 1–5 laps, 0–6 traffic
- CLEAN / BALANCED / MAYHEM race-chaos selection
- countdown/input lock
- lap/checkpoint/place/timing/finish flow
- circular minimap and compact HUD
- condition/damage presentation
- banana pickup/heal/peel loop
- rotten egg pickup/assisted throw/stink/grudge loop
- dog/cow poop hazards
- civilian traffic
- AI personalities and controlled chaos directives
- title/setup/pause/settings/restart/quit flow
- South-Asian/Bangladesh-inspired free/custom roadside presentation
- free `PN_Banana` and `PN_tropicalGroundPlants` vegetation

## Physical bike / road — FROZEN BASELINE
Do not retune without a reproducible regression:
- hidden cube chassis remains authoritative physics
- motorcycle + Manny are presentation meshes
- assisted balance/lean and lateral grip
- top speed roughly 155 km/h
- continuous flat authoritative collision floor
- segmented road presentation has no collision
- oval route radii 9000 cm × 5000 cm
- road width 1200 cm

The old invisible road-bump issue was fixed by the continuous collision floor. The user confirmed the flat road now feels flat.

## AI driving — FROZEN ACCEPTED BASELINE
The successful architectural fix separated high-level chaos/race decisions from low-level road following.

### High-level `ARIAIController`
Owns:
- personality
- desired pace / throttle / braking
- pickups and comedy items
- grudges / retaliation
- tactical intent
- stuck/recovery fallback

### Low-level `ARIRacingLineFollower`
Owns final racing execution:
- adaptive speed-scaled Pure Pursuit
- assigned race lane
- predicted lateral drift
- AI-only lane-stability assistance
- road-curvature pace envelope
- predictive rival overtaking / gap choice
- persistent traffic pass-side planning
- post-collision recovery priority

Critical rule: do not let combat/avoidance logic replace the racing path target again.

### Accepted result
The user verified:
- rivals no longer exhibit the recurring wall-to-wall oscillation
- Opponents 6 / Laps 2 / Traffic 0: rivals complete cleanly without recurring wall hits
- Opponents 6 / Laps 2 / Traffic 6: traffic collisions can occur, but normal road following stays stable
- the later professional pace / predictive overtaking pass was a good AI-mechanism improvement
- `05c2604` was explicitly reported good and is the accepted racecraft reference point

Treat this stack as stable unless a reproducible regression appears.

## Rival personalities
- BOT_01 LEECH — blocking/line annoyance
- BOT_02 HOTHEAD — impulsive retaliation/contact
- BOT_03 PETTY — item harassment
- BOT_04 GREMLIN — traps/opportunism
- BOT_05 BRAWLER — side pressure/contact
- BOT_06 TRYHARD — generally prioritizes racing

Player-facing UI should emphasize the personality names rather than internal BOT IDs.

### Chaos pacing
- first ~6 seconds after GO suppress director-created chaos
- CLEAN lowers incident chance/frequency and caps director concurrency at one
- BALANCED keeps intended defaults
- MAYHEM increases incident frequency but does not alter low-level driving competence
- simultaneous troublemakers remain bounded; the whole field must not become one permanent brawl

## Player experience / psychology direction
`docs/GAME_DESIGN_BIBLE.md` is canonical for product intent.

Research used in the design pass includes self-determination theory and GameFlow principles:
- competence — good riding should produce understandable success
- autonomy — meaningful but fast setup choices
- challenge/flow — tension without obvious cheating
- feedback — player understands what happened/who caused it/what to do next
- concentration — HUD should support the road, not dominate it
- future relatedness/social play — relevant when multiplayer is eventually considered

Do not add blatant rubber-banding/teleport cheating merely to keep races close. Future catch-up assistance, if any, must be subtle and bounded.

## Player-facing polish
Project version remains **0.1.1-demo1-polish1** unless the project config is intentionally version-bumped later.

### Race setup
- QUICK RACE wording
- opponents / laps / traffic / RACE CHAOS
- CLEAN / BALANCED / MAYHEM descriptions
- keyboard and D-pad/A menu navigation

### Settings
- graphics quality
- VSync
- Steering Feel: CALM / NORMAL / QUICK
- steering feel shapes the human player's analog input only; AI and full keyboard steering are unchanged

### HUD/results
- stale VPR/build vocabulary removed from player-facing race HUD
- close rivals identified by personality, not BOT inventory/debug strings
- final-lap emphasis
- playful outcome-specific finish messages
- opening control hints clear after the first portion of the race; controls remain visible in pause/readme

### Controller mapping
- RT throttle
- LT brake/reverse
- Left Stick steer
- LB/RB slap
- A peel/menu confirm
- B egg
- X recover
- Y race again
- Start/Menu pause
- D-pad menu navigation

Full in-game remapping is deferred, but should be considered in a later accessibility pass.

## Civilian traffic
Prototype traffic is intentionally simple for Demo 1.

Known acceptable limitation:
- bikes may still contact traffic in dense conditions

Required invariant:
- a traffic hit must not restore persistent wall-to-wall AI oscillation

## Presentation / free assets
Approved local free content includes:
- UE Third Person Manny
- Fab `MotoInteractionAnims`
- `SM_Bike`
- free `PN_Banana`
- free `PN_tropicalGroundPlants`

Runtime cooking is deliberately limited to the meshes/materials/textures actually used. Old UE4 sample template Blueprints/maps bundled in those free packs are excluded because they are irrelevant and can fail UE 5.8 cooking.

### Latest roadside identity pass — pending visual verification
`ARIDemoWorldBuilder` now creates lightweight presentation-only roadside silhouettes from Engine basic shapes:
- sparse utility poles and overhead wire rhythm
- four colorful tea-stall / roadside-shop clusters
- simple signboards used as lap landmarks
- sparse tropical tree silhouettes

All of these new pieces explicitly use `NoCollision` and sit outside the racing corridor. They must never become authoritative road collision or AI obstacles.

Humor and environment identity should remain affectionate and recognizable rather than mocking people or poverty.

## Audio
`RIAudioEvents` is asset-first and can use free imported SFX with generated fallbacks for prototype coverage.

The player bike now drives the existing `EnginePulse` and `TireSkid` events from speed/load/braking/sliding. The user tested this first slice and called the improvement acceptable. It is feedback-only and did not change physics.

Audio priority remains gameplay readability: engine/load, impacts, item throw/hit, skid/slip, countdown/GO/lap/finish and traffic horn before decorative comedy noise.

A UE-owned `SoundWaveProcedural.h` C4996 warning has appeared during successful builds and is non-fatal for the current UE 5.8 build.

## Passive playtest telemetry — pending local compile verification
`URIRaceTelemetrySubsystem` passively samples the human player's already-public state about five times per second and writes summaries through `UE_LOG`.

Current metrics:
- final place and finish time
- average / maximum speed
- overtakes and positions lost (place transitions)
- condition-loss event count and total condition lost
- approximate incident density per minute
- banana pickups / peel uses
- egg pickups / egg uses

The subsystem must remain observer-only. It must never influence steering, throttle, physics, AI, item behavior or race rules.

## Packaging
`tools/package_demo1.ps1` defaults to Shipping and:
- recursively blocks forbidden Sankool/CompoundWall content
- blocks Source/Config references to it
- records commit/dirty state
- cooks/builds/stages Win64
- requires executable/cooked containers
- writes `DEMO1_BUILD_INFO.txt`
- writes a player-facing controls/readme file
- creates a shareable ZIP

`tools/verify_demo1_package.ps1` performs static checks and can launch the newest package.

## CURRENT ACTIVE GATE
The next user intervention is justified because the latest work requires local compilation and human visual judgment.

Required next verification:
1. sync latest `dev/mvp-foundation`
2. compile `RoadsideIdiotsEditor Win64 Development` on the user's UE 5.8 machine
3. launch with `UnrealEditor.exe <uproject> -log`
4. confirm the new roadside props feel like useful environment identity rather than clutter
5. confirm none of the new props affect collision/driving
6. finish one race and confirm `RI PLAYTEST SUMMARY` lines appear in the log
7. briefly watch for any regression in the accepted AI/wall behavior

If this gate passes, continue autonomously with traffic/chaos readability and richer asset-first audio, without rewriting accepted driving systems.

## Player testing
Use `docs/PLAYER_TEST_PLAN.md` when sharing builds. Important questions include:
- Did the player feel in control?
- Did good riding feel rewarded?
- Did losses feel understandable/fair?
- Did rivals feel different?
- Was chaos readable rather than random?
- Did the player want another race?

## Deferred beyond Demo 1
- multiplayer networking
- commercial-quality motorcycle/traffic physics
- sophisticated traffic simulation
- final-quality art/audio
- additional maps/modes
- major AI rewrite unless a real bug requires it

## New-chat protocol
1. Read this file.
2. Read `docs/GAME_DESIGN_BIBLE.md`.
3. Read `docs/NEXT_MILESTONE.md`.
4. Read `docs/PLAYER_TEST_PLAN.md`.
5. Inspect current `dev/mvp-foundation` head.
6. Treat GitHub as more current than old chat text.
7. Preserve the frozen bike/road/AI racecraft baseline.
