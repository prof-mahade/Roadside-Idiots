# Roadside Idiots — Chat Handoff

Read this first in a new development chat, then inspect `dev/mvp-foundation` and `docs/NEXT_MILESTONE.md` before changing code.

## Project
Roadside Idiots is a Windows PC motorcycle racing game with believable road motion and deliberately funny, petty rider behavior.

Tagline: **The road is dangerous. The riders are worse.**

## CURRENT STATUS — DEMO 1 FUNCTIONALLY COMPLETE
On 2026-08-16 the user explicitly accepted **Demo 1 as functionally complete**.

A standalone packaged Windows build has launched outside Unreal Editor with the expected motorcycle/rider presentation, track/environment, HUD/minimap, race flow and traffic present.

Do not reopen solved foundation problems merely for theoretical improvement. The active workstream is now **Demo 1 polish/distribution**, followed by Demo 2 improvements.

## PERMANENT USER CONSTRAINT — FREE ONLY
The entire project must use only:
- assets/tools/content available to the user for $0 under the applicable license, or
- assets/models/materials/audio created by the project.

Do not recommend or plan around paid packs, subscriptions, or "buy later" content. If a suitable free asset cannot be used, make a lightweight custom replacement.

Removed and forbidden:
- SankoolArts compound/gate pack
- any `CompoundWall_Kit` copy/reference from that pack

`tools/package_demo1.ps1` enforces this before packaging.

## Branch / local environment
- stable branch: `main`
- active branch: `dev/mvp-foundation`
- local clone: `C:\GameDev\Roadside-Idiots`
- Unreal Engine 5.8.1
- imported binary presentation assets are intentionally local and may appear as untracked `Content/`

## Accepted Demo 1 gameplay baseline
Current accepted systems include:
- W/S throttle/brake-reverse, A/D steer
- Q/E slap, F banana peel, G rotten egg, R recovery
- P pause, Enter restart
- configurable 2–6 opponents, 1–5 laps, 0–6 traffic
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
The successful architectural fix was to separate high-level chaos/race decisions from low-level road following.

### High-level `ARIAIController`
Owns:
- personality
- desired pace / throttle / braking
- pickups and comedy items
- grudges / retaliation
- tactical intent
- stuck/recovery fallback

### Low-level `ARIRacingLineFollower`
Owns the final steering command:
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
- Opponents 6 / Laps 2 / Traffic 6: traffic collisions can occur, but the old wall-to-wall oscillation does not return under normal driving
- later traffic-aware changes were reported as a little better than before

Treat this stack as stable unless a packaged-build regression is reproducible.

## Rival personalities
- BOT_01 LEECH
- BOT_02 HOTHEAD
- BOT_03 PETTY
- BOT_04 GREMLIN
- BOT_05 BRAWLER
- BOT_06 TRYHARD

The chaos director limits simultaneous deliberate troublemakers so the whole field does not fight constantly. TRYHARD generally prioritizes racing.

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

Runtime cooking is deliberately limited to the meshes/materials/textures actually used. Old UE4 sample template Blueprints/maps bundled in those free packs are excluded from cooking because they are irrelevant and can fail UE 5.8 compilation.

The roadside environment also uses lightweight custom Engine-basic-shape structures so the game does not depend on paid building packs.

## Audio
`RIAudioEvents` is asset-first and can use free imported SFX with generated fallbacks for prototype coverage.

A UE-owned `SoundWaveProcedural.h` C4996 warning has appeared during successful builds and is non-fatal for the current UE 5.8 build.

## Packaging
Project version: `0.1.0-demo1-rc2`.

`tools/package_demo1.ps1` defaults to **Shipping** and:
- recursively blocks forbidden Sankool/CompoundWall content
- blocks Source/Config references to it
- records commit/dirty state
- cooks/builds/stages Win64
- requires the executable and cooked containers
- writes `DEMO1_BUILD_INFO.txt`

`tools/verify_demo1_package.ps1` performs static checks and can launch the newest package.

## CURRENT ACTIVE MILESTONE — Demo 1 polish & distribution
Do not spend the next block rewriting working gameplay foundations.

Priority:
1. remove stale internal VPR/debug text from player-facing presentation
2. polish title/setup/results/HUD spacing
3. improve traffic/hazard/item readability with free/custom content only
4. improve lightweight audio feedback/mix
5. audit package size/performance and obviously unused cooked content
6. produce a clean Shipping distribution ZIP with controls/README/known limitations
7. capture screenshots/video suitable for presenting Demo 1

After this, begin Demo 2 improvements around the frozen single-player foundation.

## Deferred beyond Demo 1
- multiplayer networking
- commercial-quality motorcycle/traffic physics
- sophisticated traffic simulation
- final-quality art/audio
- additional maps/modes
- major AI rewrite unless a real bug requires it

## New-chat protocol
1. Read this file.
2. Read `docs/NEXT_MILESTONE.md`.
3. Inspect current `dev/mvp-foundation` head.
4. Treat GitHub as more current than old chat text.
5. Preserve the frozen Demo 1 gameplay foundation.
6. Continue with polish/distribution unless the user reports a reproducible regression.
