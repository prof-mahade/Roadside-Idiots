# Next milestone — Demo 1 Presentation / Player-Test Gate

## Demo 1 status — FUNCTIONALLY COMPLETE

The standalone Windows demo has already been packaged and played outside Unreal Editor. The accepted functional baseline includes:
- configurable 2–6 AI opponents
- selectable 1–5 laps and 0–6 traffic
- CLEAN / BALANCED / MAYHEM race-chaos selection
- stable countdown / lap / place / finish / restart flow
- motorcycle + Manny presentation
- stable Pure-Pursuit racing AI without the old recurring wall oscillation
- accepted professional-pace / predictive-overtaking racecraft pass
- slap / banana peel / rotten egg / dog-poop / cow-patty loops
- civilian traffic
- minimap and race HUD
- title/setup/pause/settings/restart/quit flow
- free/custom-content-only packaging policy

Do not reopen solved foundation systems merely because they could be more sophisticated.

## Product north star

> **You are a competent motorcycle rider trying to win while surrounded by idiots.**

Drive-first, chaos-second. The player, camera, controls and road-following AI must not feel more foolish than the characters.

## Permanent constraint — FREE ONLY

Roadside Idiots may use only content/tools/assets available to the user for $0 under the applicable license, or content created by this project.

The removed SankoolArts / `CompoundWall_Kit` content must never return. Packaging preflight blocks it.

## Frozen foundation

Do not retune/rewrite without a reproducible regression:
- physical bike movement / physics baseline
- continuous flat authoritative road collision floor
- `ARIRacingLineFollower` Pure Pursuit + accepted racecraft stack
- checkpoint/lap/place/finish rules
- core camera baseline
- assisted egg targeting
- basic road dimensions / oval route geometry
- accepted player engine/skid feedback
- accepted traffic warning timing/volume

`05c2604` remains the accepted AI/racecraft reference state.

---

## Verified gameplay/readability polish

Local UE 5.8 playtests have verified:
- AI/racecraft remains stable and wall-safe
- item economy is five banana slots + three rotten-egg slots
- rotten eggs enter normal player use
- direct damage-source telemetry reaches `unknown=0`
- repeated races produce both wins and close losses, so no AI difficulty rewrite is justified
- traffic advance warning works at useful time-to-contact values
- engine and warning-horn levels are acceptable after modest volume increases
- finish celebration is kinematic / non-colliding / warning-free
- known presentation physics/collision warning regression checks pass

---

## Major presentation passes 1–3 — VERIFIED

### Pass 1
Added:
- start/finish gantry
- quarter-lap landmarks
- market / tea-stop cluster
- bus-stop / parked-bus silhouette
- pond / field section
- road edge/center/checker markings
- first rival color identity
- finish confetti

Screenshot review caught an oversized rival flag and physics-driven `NoCollision` confetti. Both were corrected rather than accepted just because logging passed.

### Pass 2 — verified at `9148a6b`
Added/corrected:
- kinematic confetti; warning spam gone
- compact rival body accents
- first traffic windows/trim/mirrors/plates
- barrier reflectors
- built-in `ROADSIDE IDIOTS`, `TEA STOP`, `BUS STOP` signage

### Pass 3 — verified at `c8024cd`
User compiled and ran in UE 5.8 with 5 opponents / 5 traffic. Runtime verification passed all required hooks and warning regression checks.

Pass 3 added:
- layered traffic shell details
- smaller integrated rival tail/fairing accents
- dark backing boards behind landmark text
- distant built-up skyline
- distant rural tree belt
- rooftop water-tank silhouettes

Observed screenshots show the world is clearly more complete than the earlier bare oval. The remaining weaknesses are art/presentation quality rather than racing-system failures.

---

# CURRENT AUTONOMOUS BLOCK 4 — PENDING LOCAL UE 5.8 COMPILE / VISUAL CHECK

Block 4 is deliberately a larger presentation batch before asking for another user verification.

## A. Sedan traffic silhouette replacement

`URITrafficVisualPolishSubsystem` now goes beyond attaching trim.

For sedan-like traffic (`TAXI`, `SUNDAY DRIVER`, `LOST DRIVER`):
- the old visible cube `CabinVisual` is hidden;
- the separate authoritative `ImpactVolume` is NOT changed;
- the visible cabin is rebuilt from:
  - body-color roof cap
  - sloped A-pillars
  - B-pillars
  - sloped C-pillars
  - front glass
  - rear glass
  - split side glass
  - hood / trunk layers
  - lamps / mirrors / bumpers / sills / plate

Large vans/microbuses retain a naturally boxier shell; CNG keeps its separate three-wheeler silhouette.

Expected traffic log now includes:
`style=tapered_shell`

## B. Road surface detail

New `URIRoadSurfaceDetailSubsystem` adds a restrained, visual-only asphalt wear layer:
- 18 dark repair patches
- paired skid streaks in four braking zones
- instanced Engine cube geometry
- only a few centimeters above the visual road
- `NoCollision`
- navigation off
- shadows off

This layer must NEVER become an authoritative road surface and must not recreate the old road-bump problem.

Expected hook:
`RI ROAD SURFACE_DETAIL patches=18 skid_streaks=40 collision=off navigation=off`

## C. Near roadside facade details

New `URIRoadsideFacadeDetailSubsystem` improves the closest blockout landmarks:
- market shop doors
- dark storefront windows
- awning valance edges
- facade separators / trim
- bus side windows
- bus belt / door outline

All are instanced, shadow-free, `NoCollision`, and outside gameplay ownership.

Expected hook:
`RI WORLD FACADE_DETAILS ... collision=off navigation=off`

## D. Distant skyline facade rhythm

`URIRoadsideBackdropSubsystem` now adds to the existing 22-building / 30-tree distant backdrop:
- dark facade window bands
- roof trims
- existing rooftop water tanks remain

This should make the skyline read as low-rise buildings instead of a row of plain cubes while keeping it deliberately low-detail and low-contrast.

## E. Mild presentation color grade

New `URIPresentationGradeSubsystem` creates one unbound `APostProcessVolume` after the race world exists.

Values are intentionally restrained:
- global contrast: `1.05`
- global saturation: `1.03`
- vignette: `0.08`

It does NOT change:
- camera transforms
- sun/skylight actors
- exposure settings
- physics
- AI
- race state

The implementation was checked against Unreal Engine 5.8 `APostProcessVolume` / `FPostProcessSettings` API.

Expected hook:
`RI PRESENTATION GRADE contrast=1.05 saturation=1.03 vignette=0.08 gameplay=unchanged`

## F. Runtime verifier

`tools/verify_polish_runtime.ps1` now requires:
- landmark layer
- world signage
- near facade details
- distant backdrop
- road markings
- road surface detail
- presentation grade
- rival identity
- item balance
- playtest start

It also reports the tapered traffic-shell hook when traffic is present and retains the physics/collision warning regression scan.

---

## Block 4 isolation check

Compared with the user-verified `c8024cd` state, Block 4 changes presentation/tooling only:
- `RIPresentationGradeSubsystem` (new)
- `RITrafficVisualPolishSubsystem`
- `RIRoadSurfaceDetailSubsystem` (new)
- `RIRoadsideFacadeDetailSubsystem` (new)
- `RIRoadsideBackdropSubsystem`
- `tools/verify_polish_runtime.ps1`
- this milestone document

Do NOT touch during this gate unless a reproducible regression appears:
- `ARIAIController`
- `ARIRacingLineFollower`
- `URIBikeMovementComponent`
- authoritative road collision floor
- checkpoints/lap/place rules
- traffic route/movement behavior
- traffic `ImpactVolume` / damage tuning

---

## Immediate gate — next justified user intervention

The next user action is justified because the remaining uncertainty is local compile/render/perception:
1. sync latest `dev/mvp-foundation`;
2. compile `RoadsideIdiotsEditor Win64 Development` on UE 5.8.1;
3. run a race with 4+ traffic;
4. judge sedan-like traffic from behind and alongside — it should no longer have the old cube cabin;
5. confirm vans/CNG still read coherently;
6. judge whether road patches/skid marks add texture without looking cluttered or producing any physical bump;
7. judge market/bus facade details and the improved distant skyline;
8. judge whether the mild color grade improves separation without looking overprocessed;
9. confirm flat-road / bike / AI / traffic collision behavior is unchanged;
10. finish and run `tools/verify_polish_runtime.ps1`;
11. watch Message Log for new warnings.

If compile fails, fix compile before visual judgment.

If one presentation element looks wrong, correct only that layer; do not reopen the frozen racing foundation.

---

## After Block 4 passes

Highest-value remaining Demo 1 work:

### Asset-first art replacement
- use verified free local vegetation/props when exact package paths are known
- never guess local asset paths
- replace procedural blockout gradually rather than importing collision-heavy packs

### Asset-first audio replacement
- replace synthetic fallback engine/load, impacts, countdown/GO/lap/finish/horn/item cues with verified free/custom assets where practical
- keep one audio owner per event category

### Player-test build
- create a fresh standalone Shipping package after the presentation gate
- run packaging preflight and runtime verification
- use `docs/PLAYER_TEST_PLAN.md` with outside players

### Demo 2 only after player-test feedback
Potential themes:
- another route/environment variation
- new hazard/race variation
- stronger rival identity
- accessibility/remapping
- side-grade/cosmetic progression
- multiplayer/social work much later

## Known acceptable Demo 1 limitations

Not current blockers:
- traffic can still collide in dense conditions
- physics are arcade/prototype rather than commercial motorcycle simulation
- one main course/mode is acceptable for Demo 1
- local procedural art/audio remains prototype quality until verified free assets replace it
- multiplayer is deferred
