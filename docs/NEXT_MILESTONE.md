# Next milestone — Demo 1 RC1 / Outside Player Test

## Current status

Roadside Idiots Demo 1 is **functionally complete enough for release-candidate testing** and the accepted driving/AI foundation remains frozen.

Current release line:

**0.1.3-demo1-rc1**

The previous Shipping stability candidate (`0.1.2-demo1-stability1`, runtime commit `ca052ab`) completed build/cook/stage/archive and passed the static package/ZIP verifier. Its standalone smoke test confirmed:
- persistent engine audio stays audible under competing sounds;
- Y during an unfinished race does nothing;
- finish-state gameplay lock works;
- Y after finish restarts the configured race;
- road/bike/AI behavior remains stable;
- packaged presentation/content renders correctly.

The one UX defect found in that standalone test was the lack of an **obvious Main Menu route**. The pause menu called the action `CHANGE RACE SETUP`, while the finish panel exposed only race-again controls. RC1 fixes that explicitly.

Do not reopen solved bike/road/AI systems without a reproducible regression.

## Product north star

> **You are a competent motorcycle rider trying to win while surrounded by idiots.**

Drive-first, chaos-second. Good riding must remain understandable and rewarding.

## Permanent FREE-ONLY constraint

Allowed:
- project-created content;
- assets/tools available to the user for $0 under their applicable license.

Forbidden permanently:
- SankoolArts content;
- `CompoundWall_Kit` or any copy/reference to it.

Packaging preflight must continue blocking those names/references.

---

# Frozen accepted foundation

Do not retune without a real regression:
- hidden physical bike chassis / assisted balance baseline;
- continuous flat authoritative road collision floor;
- oval route dimensions and road width;
- `ARIRacingLineFollower` Pure-Pursuit/racecraft stack;
- `ARIAIController` high-level personality/tactics ownership;
- checkpoint/lap/place/finish rules;
- basic camera behavior;
- traffic route/movement/impact-volume behavior;
- accepted item economy (5 banana slots / 3 egg slots).

`05c2604` remains the accepted AI/racecraft reference point.

---

# Verified presentation / gameplay state

Verified in UE 5.8 Editor and/or the packaged stability candidate:
- wall-safe AI baseline remains intact;
- old flat-road invisible bump is gone;
- item loop includes banana peel and rotten egg use;
- damage-source telemetry reaches `unknown=0` in normal tests;
- advance traffic warnings work;
- road markings / repair patches / skid streaks are presentation-only;
- start/finish, market, tea-stop, bus-stop, pond/field landmarks render;
- landmark signage and backing boards render;
- distant skyline/tree belt and rooftop water tanks render;
- facade details render;
- tapered/layered traffic presentation renders;
- integrated rival personality accents render;
- PN tropical/banana vegetation loads from approved free assets;
- finish confetti is kinematic and warning-free;
- post-process grade is mild and gameplay-neutral;
- persistent engine audio is perceptually verified in standalone Shipping;
- Y finish restart is perceptually verified;
- post-finish gameplay input lock is verified;
- road and AI remain stable in the packaged build.

---

# RC1 Main Menu UX contract

The race-setup screen is the project Main Menu. RC1 makes that relationship explicit.

## During an active race
- Esc: open Pause;
- P / Start: open/close Pause;
- B: gameplay egg on controller, or menu back when a menu owns it.

## Pause
Rows are:
1. RESUME
2. RESTART RACE
3. **MAIN MENU**
4. SETTINGS
5. QUIT GAME

`MAIN MENU` reloads the current map without the one-shot auto-start request, returning to race setup instead of automatically starting another race.

## Finish screen
- Enter / A / Y: race again with the same configured setup;
- Esc / B: **Main Menu**;
- P / Start: blocked so Pause cannot replace the result panel;
- gameplay peel/egg/slap/recovery remains blocked.

Runtime hook for a Main Menu return when logging is available:

`RI INPUT MAIN_MENU source=...`

Static contract:

`tools/verify_input_contract.ps1`

Combined contract:

`tools/verify_bugfix_contracts.ps1`

---

# Persistent engine audio architecture

The old engine implementation was a chain of short one-shot procedural pulses sharing the transient SFX path.

Current architecture:
- `URIPresentationWorldSubsystem` owns one persistent procedural engine component;
- its FIFO is continuously replenished;
- speed/throttle continuously control volume/pitch;
- engine voice gets priority override and `bShouldRemainActiveIfDropped`;
- horns/items/crashes remain transient `RIAudioEvents` sounds layered on top;
- bike movement remains physics-only.

Standalone smoke result: **engine continuity fixed**.

Static contract:

`tools/verify_audio_contract.ps1`

---

# RC1 packaging / cook hygiene

Canonical package command:

`tools/package_player_test.ps1`

The release pipeline:
- requires a clean tracked Git tree;
- permits untracked local files only under approved `Content/` / `Build/` locations;
- fingerprints branch + commit;
- requires the four approved PN vegetation assets;
- runs combined input/audio/lifecycle preflight;
- builds/cooks/stages/archives Shipping;
- removes Shipping PDBs before distribution;
- includes README, test plan, feedback form and build/preflight evidence;
- rebuilds ZIP after evidence is added;
- regenerates SHA-256;
- verifies the actual ZIP and rejects forbidden content/source/debug-symbol leakage.

The previous cook exposed avoidable missing-dependency warnings from unused free-pack sample content. RC1 narrows cook scope:
- PN Banana forces only `/Meshes/plants`; mesh dependencies bring required materials/textures;
- tropical pack forces its mesh folder without forcing every material/texture tree;
- `MotoInteractionAnims/Demo/Characters` is explicitly excluded because Roadside Idiots uses its own Manny mesh and runtime animation path rather than those demo rig assets.

Do not broaden these cook paths unless a required runtime asset is proven missing.

---

# CURRENT ACTIVE GATE — RC1 compile + Shipping package

This is the next justified local-machine intervention.

Before packaging:
1. sync latest `dev/mvp-foundation`;
2. preserve/stash only tracked Unreal-generated Config changes if they block the pull;
3. do not delete local `Content/`;
4. inspect any unexpected untracked project-root path rather than deleting it blindly;
5. run `tools/verify_bugfix_contracts.ps1`;
6. compile the Editor target once so C++ errors are caught before the longer Shipping cook.

Then run:

`tools/package_player_test.ps1`

The package must finish with:

`PLAYER TEST PACKAGE READY`

and the static verifier must report:

`STATIC PACKAGE + ZIP CHECK: PASSED`

## RC1 standalone smoke

Only RC1-specific checks need focused human attention:
1. Pause menu visibly says **MAIN MENU**;
2. selecting Pause → MAIN MENU returns to setup and does not auto-start;
3. finish panel visibly offers `ESC / B  MAIN MENU`;
4. Esc after finish returns to Main Menu;
5. B after finish returns to Main Menu;
6. Y/A/Enter after finish still restart the configured race;
7. P/Start still cannot replace results with Pause;
8. engine, road, AI and packaged visuals remain as previously accepted.

If these pass, stop internal feature/polish iteration and move to outside testers.

---

# Outside tester phase

Share the exact verified RC1 ZIP with a small mixed tester group.

Give testers:
- `README_ROADSIDE_IDIOTS.txt`;
- `PLAYER_TEST_PLAN.md` only if they are helping run a structured session;
- `PLAYER_TEST_FEEDBACK_FORM.md` after play.

Prioritize feedback in this order:
1. loss of control / broken driving;
2. unclear or unfair outcomes;
3. confusing onboarding/UI;
4. repetitive AI/chaos;
5. weak feedback/audio;
6. visual identity/polish;
7. new content.

Do not add Demo 2 content to hide Demo 1 problems.

---

# Highest-value work after first external tests

Depending on tester evidence:
- replace synthetic fallback sounds with verified free/custom audio where it materially improves clarity;
- replace the most noticeable remaining blockout environment pieces with approved free/custom assets;
- tighten HUD/rival-label clutter only if testers notice it;
- consider accessibility/remapping after core player-test feedback;
- plan a second route/content slice only after replay/fun/control scores justify moving forward.

Multiplayer remains deferred.

---

## Known acceptable Demo 1 limitations

Not blockers by themselves:
- dense traffic can still cause contact;
- motorcycle physics are arcade/prototype rather than simulation-grade;
- one main course/mode is acceptable for Demo 1;
- some procedural/blockout art remains;
- some generated fallback audio remains;
- multiplayer is not part of Demo 1.
