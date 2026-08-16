# Next milestone — Demo 1 Input / Free-Asset / Player-Test Gate

## Demo 1 status — FUNCTIONALLY COMPLETE

The standalone Windows demo has already been packaged and played outside Unreal Editor. The accepted functional baseline includes:
- configurable 2–6 AI opponents
- selectable 1–5 laps and 0–6 traffic
- CLEAN / BALANCED / MAYHEM race-chaos selection
- stable countdown / lap / place / finish flow
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

# Major presentation passes 1–4 — VERIFIED

## Pass 1
Added start/finish gantry, quarter-lap landmarks, market/tea-stop, bus stop, pond/field section, road markings, first rival identity and finish confetti.

Screenshot review caught an oversized rival flag and physics-driven `NoCollision` confetti. Both were corrected rather than accepted just because logging passed.

## Pass 2 — verified at `9148a6b`
Added/corrected:
- kinematic warning-free confetti
- compact rival body accents
- first traffic windows/trim/mirrors/plates
- barrier reflectors
- built-in `ROADSIDE IDIOTS`, `TEA STOP`, `BUS STOP` signage

## Pass 3 — verified at `c8024cd`
Added:
- layered traffic shell details
- smaller integrated rival accents
- dark backing boards behind landmark text
- distant built-up skyline / rural tree belt
- rooftop water tanks

## Pass 4 — verified at `f64fe86`
The user compiled and ran Pass 4 in UE 5.8. Runtime verification passed all required hooks and the warning regression scan.

Verified Pass 4 features:
- sedan traffic uses the tapered presentation shell (`style=tapered_shell`)
- old cube cabin no longer dominates normal sedan traffic
- 18 road repair patches + 40 skid streaks render as `NoCollision` presentation only
- close market/bus-stop facade details render
- distant backdrop has window bands / roof trims
- mild post-process grade (`contrast=1.05`, `saturation=1.03`, `vignette=0.08`) renders
- race completed normally
- damage telemetry remained clean (`unknown=0`)
- no presentation physics/collision warning regression
- bike / flat road / AI foundation remained stable

User then reported one real functional issue: **the Y race-again button did not work reliably**.

---

# CURRENT AUTONOMOUS BLOCK 5 — PENDING LOCAL COMPILE / INPUT / FREE-ASSET CHECK

This block intentionally combines the functional Y fix with the first real approved-free environment replacement and player-test tooling.

## A. Finish/restart input ownership fix

Root cause found:
- HUD advertised `ENTER / Y RACE AGAIN`;
- `ARIPlayerController` had no top-face/Y finish binding;
- `MenuConfirm()` had no post-finish branch;
- `DefaultInput.ini` also mapped Enter/Y to pawn-level `RestartRace`, creating duplicate restart ownership and a raw map-reload path that did not preserve configured race autostart behavior.

Block 5 changes:
- player controller is the effective restart/menu owner;
- Enter is an exclusive controller confirm/restart key;
- A / bottom face confirms menus and restarts after finish;
- keyboard Y and gamepad Y/top-face are finish-only quick-restart keys;
- B / right face returns from Settings and resumes from Pause;
- Start/Menu continues to pause/resume;
- Y during an unfinished race intentionally does nothing;
- restart uses `RestartConfiguredRace()` so opponent/lap/traffic/chaos setup is preserved;
- legacy `RestartRace` action mappings were removed from `DefaultInput.ini`.

Expected startup hook:
`RI INPUT FLOW confirm=ENTER/A finish_restart=ENTER/A/Y back=B pause=START restart_owner=player_controller`

Expected after pressing Y on the finish screen:
`RI INPUT FINISH_RESTART source=Y`

## B. Static input regression guard

New `tools/verify_input_contract.ps1` checks:
- no legacy `ActionName="RestartRace"` mapping exists;
- Enter binding exists;
- A confirm exists;
- B menu-back exists;
- keyboard Y + controller Y finish-restart bindings exist;
- Start pause exists;
- finish-state guard / configured-race restart path exist.

This converts the Y-button regression into a pre-package contract instead of relying on memory.

## C. First approved-free vegetation replacement

New `URIFreeVegetationSubsystem` uses exact asset paths already recognized by package preflight:
- `/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_01_04`
- `/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_05_04`
- `/Game/PN_Banana/Meshes/plants/banana_01_07`
- `/Game/PN_Banana/Meshes/plants/banana_02_05`

Presentation behavior:
- roughly 38 tropical ground-plant instances around the full course;
- roughly 10 banana-plant instances concentrated on market/rural sections;
- instanced meshes;
- outside the racing corridor;
- `NoCollision`;
- navigation off;
- no AI/road/traffic ownership.

Expected hook:
`RI FREE VEGETATION tropical=... banana=... tropical_assets=... banana_assets=... collision=off navigation=off source=approved_free`

If approved local assets are missing, the subsystem skips them instead of breaking the game.

## D. Runtime/package verification hardening

`tools/verify_polish_runtime.ps1` now requires:
- `RI INPUT FLOW`
- `RI FREE VEGETATION`
- all previously accepted presentation hooks

and reports `RI INPUT FINISH_RESTART` when a finish-restart action occurred.

`tools/package_demo1.ps1` now:
- runs `verify_input_contract.ps1` before cooking;
- records input preflight PASS in `DEMO1_BUILD_INFO.txt`;
- writes current Enter/A/B/Y/Start behavior into the packaged README;
- copies `docs/PLAYER_TEST_PLAN.md` into the package.

`tools/verify_demo1_package.ps1` now checks:
- packaged test plan exists;
- manifest records input preflight PASS;
- README contains current B/Y/Start controller guidance;
- ZIP/checksum/container/executable checks still pass.

`docs/PLAYER_TEST_PLAN.md` now includes a packaged smoke gate for:
- Enter/A/Y post-finish restart;
- Y not restarting mid-race;
- B back/resume;
- gameplay A/B/X/LB/RB actions;
- road patches having zero physical effect;
- wall-safe AI / flat-road regression;
- approved free vegetation loading.

---

## Block 5 isolation check

Compared with verified `f64fe86`, Block 5 touches only:
- `RIPlayerController` input/menu flow;
- removal of two legacy restart mappings in `DefaultInput.ini`;
- new `RIFreeVegetationSubsystem`;
- player-test/package/runtime verification tooling/docs.

Do NOT touch during this gate unless a reproducible regression appears:
- `ARIAIController`
- `ARIRacingLineFollower`
- `URIBikeMovementComponent`
- authoritative road collision floor
- checkpoint/lap/place rules
- traffic route/movement behavior
- traffic `ImpactVolume` / damage tuning

---

## Immediate gate — next justified user intervention

The next user action is justified because remaining unknowns are local compile/input/asset rendering:
1. sync latest `dev/mvp-foundation`;
2. run `tools/verify_input_contract.ps1`;
3. compile `RoadsideIdiotsEditor Win64 Development` on UE 5.8.1;
4. launch and start a configured race;
5. press keyboard/controller Y during the unfinished race: it must NOT reload;
6. finish the race;
7. press Y on the finish screen;
8. verify the race reloads and auto-starts with the same setup;
9. visually judge approved PN tropical/banana vegetation scale/density;
10. verify vegetation never enters the race corridor and causes no collision;
11. finish a race and run `tools/verify_polish_runtime.ps1`;
12. watch Message Log for new warnings.

If compile fails, fix compile before input/visual judgment.

If vegetation scale/placement is poor, tune only the presentation layer; do not reopen the frozen racing foundation.

---

## After Block 5 passes

Highest-value remaining Demo 1 work:

### Fresh standalone player-test build
- package Shipping build with current input preflight;
- run `verify_demo1_package.ps1`;
- launch packaged build, not PIE;
- run the smoke gate in `PLAYER_TEST_PLAN.md`;
- then share with a small outside tester group.

### Asset-first audio replacement
- replace synthetic fallback engine/load, impacts, countdown/GO/lap/finish/horn/item cues with verified free/custom assets where practical;
- keep one audio owner per event category.

### Environment replacement after tester feedback
- continue replacing blockout only where testers notice it;
- do not add collision-heavy decorative packs;
- preserve Bangladesh/South-Asian roadside identity.

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
- some procedural/blockout art and generated audio remain prototype quality
- multiplayer is deferred
