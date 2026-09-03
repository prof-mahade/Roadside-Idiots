# Next milestone — Demo 1 RC1 / Outside Player Test

## Current status

Roadside Idiots Demo 1 RC1 is **accepted for outside player testing**. Internal standalone Shipping verification is complete and the accepted driving/AI foundation remains frozen.

Accepted release candidate:

**0.1.3-demo1-rc1**

Source commit:

`8ab5651`

Accepted Shipping package:

`C:\GameDev\RoadsideIdiots_Packaged\RoadsideIdiots_Demo1_0.1.3-demo1-rc1_Shipping_8ab5651_20260816_072752`

Shareable ZIP:

`C:\GameDev\RoadsideIdiots_Packaged\RoadsideIdiots_Demo1_0.1.3-demo1-rc1_Shipping_8ab5651_20260816_072752.zip`

Accepted ZIP SHA-256:

`b78c5ff14e70597ef0cfcceb12aa497e390ece47dbcf1794ab5c0ec30a19a480`

Do not reopen solved bike/road/AI/audio/menu systems without a reproducible regression or clear outside-tester evidence.

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

# Accepted standalone behavior

Verified in Shipping:
- persistent engine audio remains audible underneath competing horn/item/crash sounds;
- Y during an unfinished race does nothing;
- finish-state gameplay lock works;
- Y/A/Enter after finish restart the configured race;
- P/Start cannot replace the finish result with Pause;
- Pause exposes an explicit **MAIN MENU** row;
- Pause → MAIN MENU returns to race setup without auto-starting;
- finish screen exposes `ESC / B  MAIN MENU`;
- Esc/B after finish returns to race setup without auto-starting;
- road/bike behavior remains stable;
- recurring AI wall ping-pong did not regress;
- motorcycle/rider presentation is present in the packaged build;
- PN vegetation and roadside presentation remain present in the packaged build.

The previous stability candidate exposed the lack of a clear Main Menu route; RC1 resolves that UX gap.

---

# RC1 input / menu contract

The race-setup screen is the project Main Menu.

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

Runtime hook when logging is available:

`RI INPUT MAIN_MENU source=...`

Static contract:

`tools/verify_input_contract.ps1`

---

# Persistent engine audio architecture

Current architecture:
- `URIPresentationWorldSubsystem` owns one persistent procedural engine component;
- its FIFO is continuously replenished;
- speed/throttle continuously control volume/pitch;
- engine voice gets priority override and `bShouldRemainActiveIfDropped`;
- horns/items/crashes remain transient `RIAudioEvents` sounds layered on top;
- bike movement remains physics-only.

Standalone result: **engine continuity accepted**.

Static contract:

`tools/verify_audio_contract.ps1`

---

# Cook / packaging contract

Canonical package command:

`tools/package_player_test.ps1`

The release pipeline:
- requires a clean tracked Git tree;
- permits untracked local files only under approved `Content/` / `Build/` locations;
- fingerprints branch + commit;
- requires the four approved PN vegetation assets;
- runs input/audio/cook contract preflights;
- builds/cooks/stages/archives Shipping;
- removes Shipping PDBs before distribution;
- includes README, test plan, feedback form and build/preflight evidence;
- rebuilds ZIP after evidence is added;
- regenerates SHA-256;
- verifies the actual ZIP and rejects forbidden content/source/debug-symbol leakage.

Current runtime cook roots are intentionally scoped to:
- `/Game/MotoInteractionAnims/Animations`;
- `/Game/MotoInteractionAnims/Demo/Bike/Mesh`;
- `/Game/Characters/Mannequins/Meshes`;
- `/Game/PN_Banana/Meshes/plants`;
- `/Game/PN_tropicalGroundPlants/Meshes`.

Animation skeleton/material dependencies are allowed to resolve transitively. Do not blacklist a dependency required by a forced runtime animation tree.

Static cook contract:

`tools/verify_cook_contract.ps1`

Combined preflight:

`tools/verify_bugfix_contracts.ps1`

A non-fatal PN Banana PivotPainter missing-dependency warning can still appear for unused sample material content. It is not currently a release blocker because the accepted Shipping build renders the required vegetation correctly. Do not disturb accepted vegetation merely to silence that warning unless a real packaged visual defect appears.

---

# CURRENT ACTIVE GATE — outside player feedback

Internal feature/polish iteration is paused. The next evidence should come from outside players, not more speculative tuning.

Share the exact accepted RC1 ZIP with a small mixed tester group.

Give testers:
- `README_ROADSIDE_IDIOTS.txt`;
- `PLAYER_TEST_PLAN.md` only when running a structured session;
- `PLAYER_TEST_FEEDBACK_FORM.md` after play.

Recommended minimum session:
- at least two races;
- allow the tester to choose setup without coaching;
- observe whether controls/menu/chaos are understandable;
- capture natural comments and any reproducible bugs;
- use the feedback form after the play session rather than before it.

Prioritize feedback in this order:
1. loss of control / broken driving;
2. crashes/build failures;
3. unclear or unfair outcomes;
4. confusing onboarding/UI;
5. weak feedback/audio;
6. repetitive chaos/rival readability;
7. visual identity/polish;
8. new content.

Do not add Demo 2 content to hide Demo 1 problems.

---

# Highest-value work after first external tests

Only act on evidence that repeats or clearly blocks enjoyment/usability.

Possible follow-up areas:
- replace synthetic fallback sounds with verified free/custom audio where testers notice weakness;
- tighten HUD/rival-label clutter if testers report readability problems;
- improve the most noticeable remaining blockout environment pieces with approved free/custom assets;
- consider accessibility/remapping after core feedback;
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
