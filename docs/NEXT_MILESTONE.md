# Next milestone — VPR-23B Demo Flow + Packaging Gate

VPR-21 and VPR-22A were visually accepted on the user's machine on 2026-08-15. VPR-22B/VPR-23A configurable chaos race was subsequently tested by the user and accepted as **"NOT BAD"**, so it is frozen for now unless a real regression appears.

## Permanent project constraint — FREE ONLY
Roadside Idiots must use only:
- assets/tools/content available to the user for $0 under the applicable license, or
- assets/models/materials/audio we create ourselves.

Do not recommend, plan around, purchase, or retain paid packs as a future dependency. If a suitable free asset does not exist, build a lightweight custom replacement.

The removed SankoolArts compound/gate pack must not return. `tools/package_demo1.ps1` now refuses to package if a top-level `Content` folder containing `Sankool` is detected.

## Frozen playable baseline
Do not retune these unless a real regression is observed:
- VPR-18 bike movement/physics/recovery/audio baseline
- continuous flat authoritative road collision
- checkpoint/lap/place/finish logic
- banana, rotten-egg, poop and slap mechanics
- VPR-19/20/21 environment presentation
- VPR-22A traffic silhouettes
- VPR-22B chaos-director behavior
- VPR-23A 2–6 opponents / 1–5 laps / 0–6 traffic setup

## VPR-22B / VPR-23A — PASSED FOR NOW
Current configurable race behavior:
- Opponents hard-clamped to 2–6
- Laps 1–5
- Traffic 0–6
- defaults remain 3 / 3 / 3
- scalable staggered start grid supports player + six AI
- BOT_01 LEECH, BOT_02 HOTHEAD, BOT_03 PETTY
- BOT_04 GREMLIN, BOT_05 BRAWLER, BOT_06 TRYHARD
- chaos personalities can deliberately choose other AI as sabotage/grudge targets rather than tunneling exclusively on the human
- TRYHARD remains race-focused so every rider does not share the same objective

## VPR-23B — CODED, LOCAL COMPILE/UI GATE PENDING
VPR-23B adds the remaining basic Demo 1 flow without touching accepted movement or race mechanics.

### Title / race setup
The pre-race screen now acts as the Demo 1 title/setup screen:
- Roadside Idiots title + tagline
- Opponents
- Laps
- Traffic
- START RACE
- SETTINGS
- QUIT GAME

Navigation:
- UP/DOWN select
- LEFT/RIGHT adjust
- ENTER confirm

### Pause flow
During a race:
- `P` opens/closes pause reliably in PIE
- `Esc` is also bound for packaged builds; PIE itself may reserve Escape

Pause menu:
1. RESUME
2. RESTART RACE
3. CHANGE RACE SETUP
4. SETTINGS
5. QUIT GAME

`RESTART RACE` uses a one-shot flag in `URIRaceSettingsSubsystem`, which lives in the GameInstance and therefore survives the level reload. It should restart directly with the same opponents/laps/traffic rather than forcing setup again.

`CHANGE RACE SETUP` reloads without that flag and returns to the title/setup menu.

### Graphics settings
A deliberately small settings menu uses Unreal's native `UGameUserSettings`:
- Graphics Quality: LOW / MEDIUM / HIGH / EPIC
- VSync: ON / OFF
- values are applied/saved through `ApplySettings`
- settings can be opened from title/setup or pause

No external UI/plugin dependency was introduced.

### Quit
`UKismetSystemLibrary::QuitGame` is used for the packaged game. In PIE the editor may stop PIE rather than closing Unreal Editor, which is expected.

### Packaging path
Project metadata now reports:
- version `0.1.0-demo1`

New script:
- `tools/package_demo1.ps1`

It runs UE 5.8 `RunUAT BuildCookRun` for Win64 Development with build/cook/stage/pak/compression/prerequisites/archive.

It also:
- warns if expected approved free vegetation assets are missing locally
- blocks packaging if removed SankoolArts content is detected
- creates timestamped package directories so previous packages are not overwritten

## VPR-23B local verification gate
1. close Unreal and pull current `dev/mvp-foundation`
2. compile `RoadsideIdiotsEditor Win64 Development`
3. launch PIE
4. confirm title/setup now contains START RACE, SETTINGS and QUIT GAME
5. enter SETTINGS from setup; change Quality once and toggle VSync; BACK returns to setup
6. start a normal 3-opponent race
7. press `P` and confirm the world actually pauses and PAUSED menu appears
8. RESUME and confirm movement/AI continues normally
9. pause again -> SETTINGS -> BACK and confirm it returns to PAUSED, not gameplay
10. pause -> RESTART RACE and confirm the same configured race starts directly after reload
11. pause -> CHANGE RACE SETUP and confirm title/setup returns
12. verify Enter/banana/egg/slap/recover controls still work when no menu is open
13. verify AI, traffic, minimap, hazards, road and finish flow did not regress

Do not run final packaging until this gate passes.

## After VPR-23B passes
Run the first packaged Windows smoke build:

```powershell
cd C:\GameDev\Roadside-Idiots
powershell -ExecutionPolicy Bypass -File .\tools\package_demo1.ps1
```

Then launch the packaged `.exe` outside Unreal Editor and verify:
- title/setup appears
- race starts
- Esc pause works
- restart/setup/settings work
- Quit Game closes the executable
- local free/imported assets appear
- no missing/cook errors cause visible fallbacks

## Demo 1 definition
Demo 1 is a packaged Windows solo build; multiplayer is not required.

Required before calling Demo 1 ready:
1. one coherent configurable race course
2. 2–6 selectable AI opponents
3. selectable laps and traffic density
4. race-focused and chaos-focused AI personalities
5. stable items, hazards, traffic, recovery and finish flow
6. readable minimap/HUD/countdown/results
7. title/setup/pause/settings/restart/quit flow
8. free/custom environment only
9. packaged Windows executable launches outside the editor
10. final VPR-24 bug/performance/package audit

## Remaining demo milestones
- VPR-23B: demo flow + packaging preparation — **current local gate**
- packaged Windows smoke test — immediately after VPR-23B passes
- VPR-24: final demo bug/performance/package audit

## Deferred beyond Demo 1
- multiplayer networking
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- commercial-quality final map/assets/audio
- additional maps/modes
