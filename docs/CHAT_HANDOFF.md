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

## CURRENT STATUS — DEMO 1 FUNCTIONALLY COMPLETE
On 2026-08-16 the user explicitly accepted Demo 1 as functionally complete.

A standalone packaged Windows build has launched outside Unreal Editor with the expected motorcycle/rider presentation, track/environment, HUD/minimap, race flow and traffic present.

The active branch has since received a research-informed player-experience polish batch that is **pending local Windows compile/package verification**.

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

## Accepted Demo 1 gameplay baseline
- W/S throttle/brake-reverse, A/D steer
- Q/E slap, F banana peel, G rotten egg, R recovery
- P/Esc pause, Enter restart/confirm
- Xbox-style gamepad gameplay/menu mapping added in latest polish batch
- configurable 2–6 opponents, 1–5 laps, 0–6 traffic
- latest polish adds CLEAN / BALANCED / MAYHEM race-chaos selection
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

The old invisible road-bump issue was fixed by the continuous collision floor.

## AI driving — FROZEN DEMO 1 BASELINE
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
Owns final steering:
- adaptive speed-scaled Pure Pursuit
- assigned race lane
- predicted lateral drift
- AI-only lane-stability assistance
- curvature safety speed veto
- persistent traffic pass-side planning
- post-collision recovery priority

Critical rule: do not let combat/avoidance logic replace the racing path target again.

### Accepted result
The user verified:
- Opponents 6 / Laps 2 / Traffic 0: rivals complete cleanly without recurring wall hits
- Opponents 6 / Laps 2 / Traffic 6: traffic collisions can occur, but normal road following stays stable
- later traffic-aware changes were reported as a little better than before

Treat this stack as stable unless a packaged-build regression is reproducible.

## Rival personalities
- BOT_01 LEECH — blocking/line annoyance
- BOT_02 HOTHEAD — impulsive retaliation/contact
- BOT_03 PETTY — item harassment
- BOT_04 GREMLIN — traps/opportunism
- BOT_05 BRAWLER — side pressure/contact
- BOT_06 TRYHARD — generally prioritizes racing

Player-facing UI should emphasize the personality names rather than internal BOT IDs.

### Latest chaos pacing changes — pending verification
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

## Latest player-facing polish — pending compile/package verification
Project version: **0.1.1-demo1-polish1**.

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

The environment also uses lightweight custom Engine-basic-shape structures so the game does not depend on paid building packs.

Long-term identity should grow toward recognizable South-Asian/Bangladesh-inspired roadside details (tea stalls, utility infrastructure, vegetation, local traffic silhouettes, roadside clutter) with affectionate humor rather than ridicule.

## Audio
`RIAudioEvents` is asset-first and can use free imported SFX with generated fallbacks for prototype coverage.

Audio priority should be gameplay readability: engine/load, impacts, item throw/hit, skid/slip, countdown/GO/lap/finish and traffic horn before decorative comedy noise.

A UE-owned `SoundWaveProcedural.h` C4996 warning has appeared during successful builds and is non-fatal for the current UE 5.8 build.

## Packaging
`tools/package_demo1.ps1` defaults to Shipping and now:
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
Before more feature work:
1. sync latest branch
2. compile on the user's UE 5.8 Windows machine
3. visually verify Quick Race / Chaos / Settings / cleaned HUD / finish UI
4. package Shipping build
5. verify the shareable ZIP

If this batch passes, continue autonomously with feedback/audio, environment identity, traffic/chaos readability and playtest instrumentation. Do not rewrite the accepted riding foundation.

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
7. Preserve the frozen Demo 1 driving foundation.
