# Next milestone — Demo 1 RC Packaging Gate

## Current status
The basic racing-controller crisis is no longer the active blocker.

On 2026-08-15 the user verified the new low-level racing-line follower with:
- Opponents = 6
- Laps = 2
- Traffic = 0

Result: rivals completed the run without repeatedly hitting the barriers.

A second run with Traffic = 6 was also acceptable: traffic collisions still occurred, but rivals did not resume the old left/right wall oscillation unless physically disturbed. After the VPR-25 traffic-pass/recovery pass the user reported the result was "little better than before".

Treat the current racing-line follower as a frozen Demo 1 baseline. Do not retune it merely for theoretical improvement. Reopen driving only for a reproducible regression in the packaged build.

## Permanent project constraint — FREE ONLY
Roadside Idiots must use only:
- assets/tools/content available to the user for $0 under the applicable license, or
- assets/models/materials/audio created by this project.

Do not recommend, plan around, purchase, or retain paid packs.

The removed SankoolArts / CompoundWall_Kit content must never return. `tools/package_demo1.ps1` now recursively blocks it in Content and also blocks Source/Config references.

## Frozen Demo 1 systems
Do not retune unless a real regression is observed:
- VPR-18 physical bike movement/physics baseline
- continuous flat authoritative road floor collision
- VPR-24E/VPR-25 low-level Pure-Pursuit racing-line follower
- checkpoint/lap/place/finish rules
- player controls and assisted egg behavior
- race setup: 2–6 opponents, 1–5 laps, 0–6 traffic
- title/setup/pause/settings/restart/quit flow
- minimap/HUD layout
- free vegetation/environment presentation baseline

## Current AI architecture
High-level `ARIAIController` owns:
- personality
- throttle/brake race pace
- pickups/items
- grudges and chaos directives
- recovery fallback

Low-level `ARIRacingLineFollower` owns final steering:
- speed-scaled Pure Pursuit look-ahead
- stable assigned race lane
- predicted lateral-drift recovery
- AI-only lane stabilization force
- curvature-based safety speed veto
- VPR-25 persistent traffic pass side selection
- VPR-25 collision recovery priority

The important rule is: tactical/comedy behavior must never be allowed to replace the road-following controller again.

## Demo 1 RC packaging work
`tools/package_demo1.ps1` now:
1. verifies the Unreal 5.8 project and RunUAT path
2. recursively rejects forbidden Sankool/CompoundWall content
3. rejects Source/Config references to that removed content
4. reports missing approved free vegetation assets
5. records the Git commit and dirty-working-tree state
6. runs Win64 BuildCookRun
7. requires a packaged `RoadsideIdiots.exe`
8. requires cooked `.pak/.utoc/.ucas` data
9. writes `DEMO1_BUILD_INFO.txt` into the archive

`tools/verify_demo1_package.ps1` now performs the static package check and can launch the packaged build for the manual smoke test.

Project version is now `0.1.0-demo1-rc1`.

## Local gate — do this next
Close Unreal and sync the branch, then run:

```powershell
cd C:\GameDev\Roadside-Idiots
git pull --ff-only origin dev/mvp-foundation

.\tools\package_demo1.ps1
```

If packaging succeeds, run:

```powershell
.\tools\verify_demo1_package.ps1 -Launch
```

The verifier automatically chooses the newest `RoadsideIdiots_Demo1_*` archive when `-PackagePath` is omitted.

## Manual packaged-build smoke test
### Race A — clean racing baseline
- Opponents = 6
- Laps = 2
- Traffic = 0
- verify countdown/input lock
- verify rivals complete clean laps without recurring barrier oscillation
- verify lap/place/minimap/finish

### Race B — full prototype stress
- Opponents = 6
- Laps = 2
- Traffic = 6
- verify traffic overtaking/slowing is at least acceptable
- verify a collision can be recovered from
- verify traffic contact does not cause persistent wall-to-wall oscillation
- test Q/E slap, F peel, G egg, R recovery, P pause and Enter restart
- verify banana/egg/poop effects, audio, HUD and free vegetation
- finish the race, start another race, then quit normally

## Demo 1 definition
Demo 1 is a packaged Windows solo build. Multiplayer is not required.

Required before calling Demo 1 ready:
1. coherent configurable race course
2. 2–6 selectable AI opponents
3. selectable laps and traffic
4. competent racing AI with occasional chaos
5. stable egg / peel / slap / poop mechanics
6. stable traffic, recovery and finish flow
7. readable HUD/minimap/results
8. title/setup/pause/settings/restart/quit
9. free/custom assets only
10. packaged Windows executable launches outside the editor
11. packaged two-race smoke test passes

## Deferred beyond Demo 1
- multiplayer networking
- final commercial motorcycle/traffic physics
- final-quality maps/assets/audio
- sophisticated traffic simulation
- additional maps/modes
- major AI architecture rewrites unless required by a real bug
