# Roadside Idiots

**Roadside Idiots** is a PC motorcycle racing/combat game built around believable riding, chaotic crashes, petty NPC behavior, and emergent comedy.

> **Design promise:** realistic-looking road chaos caused by ridiculous human behavior.

## Current phase

**Phase 0 — Project Blueprint / Architecture Freeze**

No gameplay code should be treated as canonical until the project blueprint is reviewed.

## Start here

- [`docs/PROJECT_BLUEPRINT.md`](docs/PROJECT_BLUEPRINT.md) — canonical source of truth for the whole project
- [`docs/CHAT_HANDOFF.md`](docs/CHAT_HANDOFF.md) — concise context for a new ChatGPT conversation
- [`docs/MVP_SCOPE.md`](docs/MVP_SCOPE.md) — exact first-playable scope and definition of done
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) — system boundaries, ownership, data flow, and class plan
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — staged development after the MVP
- [`docs/DECISIONS.md`](docs/DECISIONS.md) — architectural/design decisions that should not be casually reversed
- [`docs/FEATURE_BACKLOG.md`](docs/FEATURE_BACKLOG.md) — future ideas kept out of the MVP

## Core MVP loop

`Start race → ride → race AI → attack / get attacked → crash / recover → finish → restart`

The MVP exists to answer one question:

> **Is riding, fighting, crashing, and dealing with irritating AI fun enough that the player wants another race?**

## Development principles

1. **Playable before pretty.**
2. **MVP before expansion.**
3. **Systems create comedy; scripts do not manufacture every joke.**
4. **Gameplay state and presentation stay separate.**
5. **Data-driven tuning instead of hard-coded content.**
6. **Single-player first, multiplayer-aware architecture from day one.**
7. **Free/legal tools and assets only during the prototype phase.**
8. **The repository is the source of truth, not chat history.**

## Working title

**Roadside Idiots**

Working tagline:

> **The road is dangerous. The riders are worse.**
