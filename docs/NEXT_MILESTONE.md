# Next milestone — Demo 1 Polish / Player Experience Gate

## Demo 1 status — FUNCTIONALLY COMPLETE

The standalone Windows demo has already been packaged and played outside Unreal Editor. The accepted functional baseline includes:
- configurable 2–6 AI opponents
- selectable 1–5 laps and 0–6 traffic
- stable countdown / lap / place / finish / restart flow
- motorcycle + Manny presentation in packaged builds
- stable Pure-Pursuit-based racing AI without the old recurring wall oscillation
- accepted professional-pace / predictive-overtaking racecraft pass
- prototype traffic passing/recovery at an acceptable level
- slap / banana peel / rotten egg / dog-poop / cow-patty gameplay loops
- minimap and race HUD
- title/setup/pause/settings/restart/quit flow
- free vegetation/environment baseline
- free/custom-content-only packaging policy

Do not reopen foundation systems merely because they could be theoretically more sophisticated.

---

## Product north star

Read `docs/GAME_DESIGN_BIBLE.md` before adding meaningful features.

The permanent fantasy is:

> **You are a competent motorcycle rider trying to win while surrounded by idiots.**

The game is **drive-first, chaos-second**. The player, camera, controls and road-following AI must not feel more foolish than the characters.

`docs/PLAYER_TEST_PLAN.md` defines how future builds should be evaluated with real players.

---

## Permanent project constraint — FREE ONLY

Roadside Idiots may use only:
- content/tools/assets available to the user for $0 under the applicable license, or
- content created by this project.

Do not recommend, plan around, buy or retain paid packs.

The removed SankoolArts / CompoundWall_Kit content must never return. `tools/package_demo1.ps1` recursively blocks it in Content and Source/Config references.

---

## Frozen foundation

Do not retune/rewrite without a reproducible regression:
- physical bike movement/physics baseline
- continuous flat authoritative road collision floor
- `ARIRacingLineFollower` Pure-Pursuit + accepted professional racecraft stack
- checkpoint/lap/place/finish rules
- core camera baseline
- assisted egg targeting behavior
- basic road dimensions/route geometry

The user explicitly reported `05c2604` as a good AI-mechanism improvement. Preserve it as the accepted racecraft reference state.

---

## Accepted recent polish

Project version remains **0.1.1-demo1-polish1** unless deliberately changed in project config.

### Player choice / race pacing
- race setup includes `RACE CHAOS`: CLEAN / BALANCED / MAYHEM
- CLEAN reduces director-created trouble
- BALANCED is the intended default
- MAYHEM raises incident frequency without changing the low-level racing controller
- director-created chaos waits about 6 seconds after GO
- simultaneous deliberate troublemakers remain capped

### Input / accessibility comfort
- Xbox-style gamepad gameplay mappings
- D-pad + A menu navigation
- Start/Menu pauses
- player-only Steering Feel: CALM / NORMAL / QUICK
- steering feel shapes analog response only; keyboard full steering and AI driving are unchanged

### HUD / presentation
- stale VPR/build text removed from player-facing race HUD
- nearby rivals are identified primarily by personality instead of BOT numbers
- finish message is playful and outcome-specific
- final lap gets emphasis
- opening control hints clear after the first portion of the race

### Feedback/audio
- asset-first `RIAudioEvents` remains the long-term hook
- player `EnginePulse` is now driven from speed/throttle load
- player `TireSkid` is driven from hard braking/lateral sliding
- the user tested this first driving-audio slice and called the improvement acceptable
- no physics or AI tuning values were changed by this audio pass

### Distribution
`tools/package_demo1.ps1`:
- defaults to Shipping
- runs free-only preflight
- builds/cooks/stages Win64
- writes build metadata
- writes a player-facing controls/readme file
- creates a shareable ZIP automatically

---

## Current autonomous batch — pending local compile / human visual check

### Environment identity
`ARIDemoWorldBuilder` now adds a collision-free custom roadside silhouette pass using only Engine basic shapes:
- sparse utility poles and overhead wire rhythm
- four colorful tea-stall / roadside-shop clusters
- simple roadside signboards / landmarks
- sparse tropical tree silhouettes

Every new piece explicitly uses `NoCollision` and sits outside the race corridor. This pass must remain presentation-only.

### Passive playtest telemetry
`URIRaceTelemetrySubsystem` is an auto-instanced tickable world subsystem that passively observes public race/player state. It samples at about 5 Hz and logs:
- finish place/time
- average/max speed
- overtakes and positions lost
- condition-loss events / total condition lost
- approximate incident density
- banana pickups / peel uses
- egg pickups / egg uses

It must remain observer-only and never affect gameplay.

---

## Immediate gate — local compile + visual/gameplay verification

The next user intervention is needed here because the new environment is fundamentally a visual/player-experience change.

Required checks:
1. C++ build succeeds on UE 5.8.1.
2. The new roadside silhouettes are visible.
3. They make the course feel more like a recognizable roadside environment rather than a generic test oval.
4. They do not feel excessively cluttered or block important road/traffic/hazard reading.
5. No new decorative piece collides with the bike.
6. Accepted AI wall safety / racecraft still feels unchanged.
7. Finish one race and confirm `RI PLAYTEST SUMMARY`, `RESULT`, `RACECRAFT`, and `ITEMS` lines appear in the Unreal log.

If this passes, do not reopen the environment geometry immediately. Move on to traffic/chaos readability and richer asset-first audio.

---

## Next autonomous work after this gate

### A. Traffic / chaos readability
- improve advance readability of traffic and hazards
- preserve committed passing and collision recovery
- refine rival personality expression without increasing constant aggression
- add variety through blocking/traps/opportunism rather than endless slapping

### B. Richer asset-first audio
- replace the most synthetic fallback cues with suitable free/custom assets when practical
- prioritize impact, countdown/GO/lap/finish, horn and item feedback
- avoid constant comedy noise that masks useful events

### C. Telemetry expansion only when useful
Possible additions after real playtests justify them:
- collisions by type
- explicit recoveries
- item hit rates
- finish gap
- restart/quit points

Do not collect metrics merely because they are easy to count; use them to answer concrete playtest questions.

### D. Demo 2 only after polish gate
Candidate Demo 2 themes:
- another route/environment variation
- stronger rival identity
- new race/hazard variation
- progression through meaningful side-grades/cosmetics rather than grind
- deeper options/accessibility including remapping
- local/online social features much later

---

## Known acceptable Demo 1 limitations

Not current blockers:
- traffic can still collide in dense conditions
- physics are arcade/prototype, not commercial motorcycle simulation
- environment/art/audio are still prototype quality
- one main course/mode is acceptable for Demo 1
- AI tactics can expand later
- multiplayer is deferred

The next goal is **more readable, more distinctive, more replayable**, not “more systems at any cost.”
