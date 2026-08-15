# Roadside Idiots — Chat Handoff

Read this first in a new development chat, then inspect `dev/mvp-foundation` and `docs/NEXT_MILESTONE.md` before changing code.

## Project
Roadside Idiots is a Windows PC motorcycle racing game with believable road motion and deliberately funny, petty rider behavior. Long term it may become multiplayer; Demo 1 is a solo Windows build.

Tagline: **The road is dangerous. The riders are worse.**

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
- Visual Studio Community 2026, Game Development with C++
- imported binary presentation assets are intentionally local and may appear as untracked `Content/`

## Accepted gameplay baseline
The user has repeatedly accepted the prototype as playable/not bad for the current stage.

Current systems include:
- W/S throttle/brake-reverse, A/D steer
- Q/E slap, F banana peel, G rotten egg, R recovery
- P pause, Enter restart
- configurable 2–6 opponents, 1–5 laps, 0–6 traffic
- countdown/input lock
- lap/checkpoint/place/timing/finish flow
- circular minimap and compact HUD
- condition/damage and bandage presentation
- banana pickup/heal/peel loop
- rotten egg pickup/assisted throw/stink/grudge loop
- dog/cow poop hazards
- civilian traffic
- AI personalities and controlled chaos directives
- title/setup/pause/settings/restart/quit flow
- South-Asian/Bangladesh-inspired free/custom roadside graybox presentation
- free `PN_Banana` and `PN_tropicalGroundPlants` vegetation

## Physical bike / road — frozen
Do not retune without a reproducible regression:
- hidden cube chassis remains authoritative physics
- motorcycle + Manny are presentation meshes
- assisted balance/lean and lateral grip
- top speed roughly 155 km/h
- continuous flat authoritative collision floor
- segmented road presentation has no collision
- oval route radii 9000 cm × 5000 cm
- road width 1200 cm
- route/barrier reference now uses 80 points/segments

The old invisible road-bump issue was fixed by the continuous collision floor.

## AI driving — current accepted architecture
A long series of VPR-24 tests exposed a persistent left/right wall-oscillation problem. The successful architectural fix was to separate high-level chaos/race decisions from low-level road following.

### High-level `ARIAIController`
Owns:
- personality
- desired pace / throttle / braking
- pickups and comedy items
- grudges / retaliation
- tactical intent
- stuck/recovery fallback

### Low-level `ARIRacingLineFollower`
Owns the final steering command after the high-level AI tick:
- adaptive speed-scaled Pure Pursuit
- assigned race lane
- predicted lateral drift
- AI-only lane-stability assistance
- curvature safety speed veto
- VPR-25 persistent traffic pass-side planning
- VPR-25 post-collision recovery priority

Critical rule: do not let combat/avoidance logic replace the racing path target again.

### Latest local result
On 2026-08-15 the user ran:
- Opponents 6 / Laps 2 / Traffic 0: rivals completed without repeatedly hitting walls
- Opponents 6 / Laps 2 / Traffic 6: traffic impacts still happened, but wall following remained stable unless a bike was physically disturbed
- after VPR-25 the user reported it was "little better than before"

Treat this driving stack as the Demo 1 baseline. Further AI sophistication should be layered around it, not rewrite it before packaging.

## Rival personalities
- BOT_01 LEECH
- BOT_02 HOTHEAD
- BOT_03 PETTY
- BOT_04 GREMLIN
- BOT_05 BRAWLER
- BOT_06 TRYHARD

The chaos director limits simultaneous deliberate troublemakers so the whole field does not fight constantly. TRYHARD generally prioritizes racing.

## Civilian traffic
Prototype traffic uses route-driven kinematic motion with overlap impact reactions. It is intentionally simple for Demo 1.

Known acceptable limitation:
- bikes may still contact traffic in dense conditions

Required behavior:
- AI should attempt a committed pass or slow before a clear collision
- a traffic hit must not restore the old persistent wall-to-wall oscillation

## Presentation / free assets
Approved local free content includes:
- UE Third Person Manny
- Fab `MotoInteractionAnims` used in the project
- `SM_Bike`
- free `PN_Banana`
- free `PN_tropicalGroundPlants`

Approved free vegetation references used by the prototype:
- `/Game/PN_Banana/Meshes/plants/banana_01_07.banana_01_07`
- `/Game/PN_Banana/Meshes/plants/banana_02_05.banana_02_05`
- `/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_01_04.tropicalPlant_01_04`
- `/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_05_04.tropicalPlant_05_04`

The roadside environment also uses lightweight custom Engine-basic-shape structures so the game does not depend on paid building packs.

## Audio
`RIAudioEvents` is asset-first:
- checks `/Game/Audio/SFX/SFX_<Event>.SFX_<Event>`
- free imported SFX can override generated fallbacks
- generated fallbacks cover countdown/GO/lap/finish, impacts, honk, eggs, peel, poop, pickups, engine pulse and tire skid

A UE-owned `SoundWaveProcedural.h` C4996 warning has appeared during successful builds and is non-fatal for the current UE 5.8 build.

## CURRENT ACTIVE GATE — Demo 1 RC package
Project version: `0.1.0-demo1-rc1`

Do not start another large editor feature pass before this gate is attempted.

Package:
```powershell
cd C:\GameDev\Roadside-Idiots
git pull --ff-only origin dev/mvp-foundation
.\tools\package_demo1.ps1
```

Then verify/launch the newest package:
```powershell
.\tools\verify_demo1_package.ps1 -Launch
```

The packaging script:
- recursively blocks forbidden Sankool/CompoundWall content
- blocks Source/Config references to it
- records commit/dirty state
- cooks/builds/stages Win64
- requires the executable and cooked containers
- writes `DEMO1_BUILD_INFO.txt`

## Packaged smoke-test gate
Race A:
- Opponents 6
- Laps 2
- Traffic 0
- confirm clean AI racing, lap/place/minimap/finish

Race B:
- Opponents 6
- Laps 2
- Traffic 6
- confirm traffic pass/slowing is acceptable
- confirm collision recovery
- test slap/peel/egg/recovery/pause/restart
- confirm hazards, sound, HUD and free vegetation
- finish, start another race, then quit normally

If these pass outside the editor, Demo 1 can be called ready for its current prototype scope.

## Demo 1 scope
Required:
1. coherent configurable course
2. 2–6 AI opponents
3. selectable laps and traffic
4. competent racing AI with occasional chaos
5. stable slap/peel/egg/poop loops
6. stable traffic/recovery/finish flow
7. readable HUD/minimap/results
8. title/setup/pause/settings/restart/quit
9. free/custom assets only
10. packaged Windows executable launches outside Unreal Editor
11. final packaged smoke test passes

Deferred beyond Demo 1:
- multiplayer networking
- commercial-quality motorcycle/traffic physics
- sophisticated traffic simulation
- final-quality art/audio
- additional maps/modes
- major AI rewrite unless a real packaged-build bug requires it

## New-chat protocol
1. Read this file.
2. Read `docs/NEXT_MILESTONE.md`.
3. Inspect current `dev/mvp-foundation` head.
4. Treat GitHub as more current than old chat text.
5. Continue from the package/smoke-test gate unless the user reports a reproducible regression.
