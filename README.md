# Roadside Idiots

**Roadside Idiots** is a Windows PC motorcycle racing/combat game about competent riding, petty rivals, traffic pressure and emergent roadside comedy.

> **The road is dangerous. The riders are worse.**

## Current status

**Demo 1 is functionally complete.**

The project can build and package a standalone Windows race with:
- configurable 2–6 AI opponents
- 1–5 laps
- 0–6 civilian traffic vehicles
- CLEAN / BALANCED / MAYHEM rival-chaos styles
- motorcycle combat: left/right slap, banana peel and assisted rotten egg
- dog/cow road hazards
- condition/damage/recovery
- AI personalities
- stable Pure-Pursuit-based racing-line following
- minimap, race HUD, countdown, lap/place/time and finish flow
- title/setup, pause, settings, restart and quit
- keyboard and Xbox-style gamepad controls
- free/custom presentation content only
- Shipping Windows packaging with a player README, shareable ZIP and SHA-256 checksum

Current project version: **0.1.1-demo1-polish1**

## Core fantasy

> **You are a competent motorcycle rider trying to win while surrounded by idiots.**

This is the main design filter. Driving must feel trustworthy first; comedy and chaos should create readable problems around that skill rather than make the game itself feel random or unfair.

## Core loop

`Set up race → ride fast → read traffic/rivals → race / dodge / attack / trap → recover → finish → race again`

## Controls

### Keyboard
- `W` accelerate
- `S` brake / reverse
- `A / D` steer
- `Q / E` slap left / right
- `F` drop banana peel
- `G` throw rotten egg
- `R` recover
- `P / Esc` pause
- `Enter` confirm / race again
- Arrow keys navigate menus

### Xbox-style gamepad
- `RT` accelerate
- `LT` brake / reverse
- Left Stick steer
- `LB / RB` slap left / right
- `A` drop banana peel / confirm menus
- `B` throw rotten egg
- `X` recover
- `Y` race again
- Menu/Start pause
- D-pad navigate menus

Settings include graphics quality, VSync and player-only **Steering Feel** (`CALM / NORMAL / QUICK`).

## Race chaos

- **CLEAN** — rivals mostly focus on racing; deliberate trouble is uncommon.
- **BALANCED** — intended default mix of racing and petty chaos.
- **MAYHEM** — deliberate rival incidents happen more often, while the accepted low-level driving controller remains unchanged.

## Start here

- [`docs/GAME_DESIGN_BIBLE.md`](docs/GAME_DESIGN_BIBLE.md) — product fantasy, audience psychology and permanent design rules
- [`docs/PLAYER_TEST_PLAN.md`](docs/PLAYER_TEST_PLAN.md) — how to test fairness, control, replay desire and chaos readability with players
- [`docs/ENVIRONMENT_STYLE_GUIDE.md`](docs/ENVIRONMENT_STYLE_GUIDE.md) — respectful Bangladesh/South-Asian-inspired world identity and free/custom environment direction
- [`docs/NEXT_MILESTONE.md`](docs/NEXT_MILESTONE.md) — current development priority and verification gate
- [`docs/CHAT_HANDOFF.md`](docs/CHAT_HANDOFF.md) — current state for a new development conversation
- [`docs/PROJECT_BLUEPRINT.md`](docs/PROJECT_BLUEPRINT.md) — original architecture/product blueprint
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) — system boundaries and ownership
- [`docs/DECISIONS.md`](docs/DECISIONS.md) — decisions that should not be casually reversed
- [`docs/FEATURE_BACKLOG.md`](docs/FEATURE_BACKLOG.md) — deferred ideas

## Build / package

Local project path used during development:

```powershell
C:\GameDev\Roadside-Idiots
```

Build a shareable Shipping package:

```powershell
cd C:\GameDev\Roadside-Idiots
.\tools\package_demo1.ps1
```

Verify and launch the newest package:

```powershell
.\tools\verify_demo1_package.ps1 -Launch
```

The packaging preflight rejects known forbidden paid/licensing-risk content, builds/cooks/stages the Win64 game, writes build metadata and player controls, creates a versioned shareable ZIP, and writes/verifies its SHA-256 checksum.

## Permanent project rules

1. **Driving competence before chaos.**
2. **Systems create comedy; arbitrary randomness does not replace readable cause/effect.**
3. **Do not let combat/avoidance logic overwrite the accepted low-level racing-line follower.**
4. **Single-player quality first; keep architecture multiplayer-aware for later.**
5. **Free/custom content only.** No paid pack may become a project dependency.
6. **South-Asian/Bangladesh-inspired roadside identity should grow through respectful, recognizable details rather than generic race-track decoration.**
7. **The repository is the source of truth, not old chat history.**

## Next direction

Demo 1 polish now focuses on player-facing quality rather than foundation survival:
- clearer menus/HUD and rival identity
- controller/accessibility comfort
- richer free/custom roadside identity
- stronger audio/impact feedback
- traffic and chaos readability
- distribution/smoke-test quality

After that, Demo 2 can add more race variety, environments, modes, rival depth and progression without destabilizing the proven Demo 1 core.
