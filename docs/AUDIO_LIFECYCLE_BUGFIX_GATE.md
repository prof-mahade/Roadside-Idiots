# Roadside Idiots — Audio / Finish-Lifecycle Bugfix Gate

## Trigger

After the `ca7b206` editor verification, the user reported that the motorcycle engine sound appeared to go away when other sounds occurred and asked for a broader bug sweep before moving on.

## Root cause found — engine audio

The player engine was not a continuous channel. `URIPresentationWorldSubsystem` repeatedly fired short `EnginePulse` one-shots through the same generic transient helper used by horns, impacts, items and race cues.

That architecture made the foundational engine note vulnerable to perceptual gaps/masking when louder transient events fired.

## Pending fix

`URIPresentationWorldSubsystem` now owns one persistent procedural engine channel:
- attached to the human bike chassis;
- buffered through `USoundWaveProcedural`;
- volume and pitch follow speed/throttle continuously;
- high audio priority override;
- remains active if dropped by prioritization;
- transient events remain owned by `RIAudioEvents`;
- bike movement remains physics-only;
- nearby rival crash sounds are rate-limited so a pack pile-up cannot create a same-frame crash chorus.

Expected runtime hook:

`RI AUDIO ENGINE channel=persistent_procedural priority=4 remain_active_if_dropped=1 transient_owner=RIAudioEvents`

## Broader bug sweep — human finish lifecycle

A second real lifecycle bug was found: `ARIBikePawn::IsRaceInputEnabled()` only checked whether the race had started. It therefore remained true after the human participant had finished.

The pending fix keeps accepted AI behavior unchanged and blocks post-finish gameplay only for the human rider.

After the human finishes:
- throttle/brake/steer are explicitly cleared;
- peel / egg / slap / manual recover cannot execute;
- new crash damage is blocked;
- traffic impact was already gated and remains gated;
- banana and egg pickups ignore the finished human;
- banana peel hazards ignore the finished human;
- rotten egg projectiles ignore the finished human;
- poop hazards ignore the finished human;
- rival slap interactions ignore the finished human.

The bike may still coast physically. This is intentional presentation/physics continuity, not active gameplay input.

## Restart ownership cleanup

The old pawn-level `RestartRace` binding and method have been removed entirely. Restart remains owned by `ARIPlayerController`, which preserves configured race settings on restart.

## Regression tooling

Added/expanded:
- `tools/verify_input_contract.ps1`
  - no `RestartRace` config mapping;
  - no bike-pawn restart binding/method;
  - controller Enter/A/Y/B/Start contract;
  - human finish lifecycle guard;
- `tools/verify_audio_contract.ps1`
  - persistent engine component/wave exists;
  - attached/buffered channel exists;
  - priority and remain-active protection exist;
  - presentation no longer fires one-shot `EnginePulse`;
  - bike movement contains no audio ownership;
- `tools/verify_bugfix_contracts.ps1`
  - runs both static contracts;
- `tools/verify_polish_runtime.ps1`
  - now requires the persistent-engine runtime hook.

## Frozen systems not changed

This bugfix gate does NOT retune or rewrite:
- `ARIAIController`;
- `ARIRacingLineFollower` / Pure Pursuit;
- `URIBikeMovementComponent` physics;
- authoritative flat road collision;
- checkpoint/lap/place rules;
- traffic movement;
- traffic collision/damage tuning.

## Required local verification

The next justified user intervention is local UE 5.8 compile/runtime because the pending engine channel uses C++ audio APIs and must be heard under a real game mix.

Verify:
1. static bugfix contract preflight passes;
2. `RoadsideIdiotsEditor Win64 Development` compiles;
3. engine remains continuously audible while horn, slap, egg, traffic hit and lap/finish sounds occur;
4. engine does not audibly disappear after a transient cue;
5. before finish, Y does nothing;
6. after finish, gameplay buttons do not change the bike/inventory behind the result screen;
7. specifically press Y on the finish screen and confirm the same configured race auto-restarts;
8. runtime verifier sees the persistent engine hook and no known warning regression.

If this gate passes, stop editor micro-polish and move to a fresh standalone Shipping/player-test package.
