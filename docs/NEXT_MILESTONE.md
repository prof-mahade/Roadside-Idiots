# Next milestone — Demo 1 Stability Release / Standalone Player Test

## Current status

Roadside Idiots Demo 1 is **functionally complete** and the accepted driving/AI foundation is frozen.

Current release line:

**0.1.2-demo1-stability1**

The latest Editor/runtime work has already verified the major presentation passes, approved free vegetation, source-aware telemetry, controller/menu ownership, and the new persistent engine audio channel at the technical-hook level.

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

Verified in local UE 5.8 playtests:
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
- known presentation physics/collision/asset-loading warning scans pass.

---

# Stability bugfix batch — technically verified, final human smoke still required

## Controller / restart ownership

Current intended contract:
- Enter: menu confirm / race again after finish;
- A: menu confirm / peel in gameplay / race again after finish;
- Y: finish-only quick restart;
- B: egg in gameplay / menu back / resume;
- Start: pause/resume during an unfinished race;
- after finish, Start/P/Esc must NOT replace the result screen with Pause.

Restart ownership is now the player controller only. The old pawn-level `RestartRace` mapping/method is removed.

After the human finishes:
- residual throttle/brake/steering is zeroed;
- peel/egg/slap/recovery input is blocked;
- new pickup/hazard/combat effects are blocked from changing the finished player;
- physical bike coasting remains possible;
- accepted AI finish/coast behavior is intentionally unchanged.

Static contract:

`tools/verify_input_contract.ps1`

Combined contract:

`tools/verify_bugfix_contracts.ps1`

## Persistent engine audio

The old engine implementation was a chain of short one-shot procedural pulses sharing the transient SFX path.

Current architecture:
- `URIPresentationWorldSubsystem` owns one persistent procedural engine component;
- its FIFO is continuously replenished;
- speed/throttle continuously control volume/pitch;
- engine voice gets priority override and `bShouldRemainActiveIfDropped`;
- horns/items/crashes remain transient `RIAudioEvents` sounds layered on top;
- bike movement remains physics-only.

Static contract:

`tools/verify_audio_contract.ps1`

Runtime hook observed successfully:

`RI AUDIO ENGINE channel=persistent_procedural priority=4 remain_active_if_dropped=1 transient_owner=RIAudioEvents`

The remaining question is perceptual: **does the engine actually remain audibly continuous underneath competing sounds?**

---

# Release engineering completed for the next gate

## New canonical package command

Use:

`tools/package_player_test.ps1`

Do not use the old base packager as the primary player-test entry point.

The hardened player-test pipeline now:
- requires a clean tracked Git tree;
- deliberately allows untracked approved local `Content/`;
- fingerprints branch + full/short commit;
- requires the four approved PN vegetation assets;
- runs the combined input/audio/lifecycle preflight;
- invokes the existing UE Shipping cook/archive pipeline;
- adds `PLAYER_TEST_BUILD_INFO.txt`;
- adds `BUGFIX_PREFLIGHT.txt`;
- includes `PLAYER_TEST_FEEDBACK_FORM.md`;
- rebuilds the final ZIP after evidence is added;
- regenerates SHA-256;
- automatically runs the static package/ZIP verifier.

## Package verifier

`tools/verify_demo1_package.ps1` now validates:
- packaged executable;
- cooked `.pak` / `.utoc` / `.ucas` files;
- README;
- player-test plan;
- player-test feedback form;
- reproducibility evidence;
- combined bugfix preflight evidence;
- ZIP existence and SHA-256;
- required files inside the actual ZIP;
- no forbidden Sankool/CompoundWall names in loose package or ZIP;
- no accidental C++ source/header files in the distributable.

Shipping runtime logging is **not assumed**. Human standalone testing remains authoritative for audio continuity and control feel.

---

# CURRENT ACTIVE GATE — build the real Shipping player-test package

This is now the next justified user intervention.

## Step A — sync current branch

Sync latest `dev/mvp-foundation` and ensure the tracked working tree is clean.

Untracked imported `Content/` is expected and must not be deleted.

If Unreal regenerated tracked Config files, back them up/stash only those tracked config changes rather than resetting or deleting Content.

## Step B — run the canonical package pipeline

Run:

`tools/package_player_test.ps1`

Default configuration is Shipping.

The pipeline should finish with:

`PLAYER TEST PACKAGE READY`

and print:
- package directory;
- ZIP path;
- SHA-256;
- source branch/commit;
- feedback form path.

## Step C — standalone human smoke gate

Launch the **packaged executable**, not PIE.

Required checks:
1. setup/menu works with keyboard and Xbox-style controller;
2. Y during an unfinished race does nothing;
3. engine remains continuously audible underneath horn/item/crash sounds;
4. A/B/X/LB/RB/Start gameplay controls work during the race;
5. finish a race;
6. Start/P/Esc after finish does not replace the result screen with Pause;
7. peel/egg/slap/recovery controls are blocked after finish;
8. Y after finish restarts the same configured race and auto-starts it;
9. visible road repair/skid detail creates zero physical bump;
10. busy traffic does not restore AI wall ping-pong;
11. PN vegetation / traffic shells / roadside identity render correctly.

If any item fails, fix that specific regression before outside testing.

---

# After standalone smoke passes

Share the exact verified ZIP with a small mixed tester group.

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
