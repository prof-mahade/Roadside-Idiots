# Roadside Idiots — Chat Handoff

This file is the fastest way for a new ChatGPT session to understand the project.

## Project
Roadside Idiots is a Windows PC motorcycle racing/combat game. The visual world should look believable and close to realistic, while riders behave in petty, emotional, funny, irrational ways.

Core identity: **realistic-looking roads, ridiculous people.**

Working tagline: **The road is dangerous. The riders are worse.**

## Current priority
Build a small SOLO playable MVP first. Do not expand the feature set until the core loop works and feels fun.

Core MVP loop:
`Start race -> ride -> race AI -> attack/get attacked -> crash/recover -> finish -> restart`

## MVP scope
- 1 compact race route
- 1 motorcycle type
- player rider
- 3-4 AI riders
- basic civilian traffic
- throttle, brake, steering, assisted lean/balance
- basic slap/punch and left/right kick
- health/damage
- crash detection and rider ragdoll
- recovery/reset
- checkpoints, race progress, finish
- basic HUD
- simple AI retaliation/aggression
- basic hit/crash audio and funny reactions

## Not in MVP
Do not implement yet: online multiplayer, open world, progression, shops, many bikes, many weapons, police, advanced injury/bandage visuals, banana/rotten egg/poop hazards, weather, advanced personality/grudge network, replay tools, story campaign.

These are future backlog items only.

## Architecture rules
1. Gameplay logic and presentation are separate.
2. Single-player first, but core state should remain multiplayer-aware.
3. Bike and rider are separate concepts.
4. AI intent is separate from AI driving/control.
5. Prefer event-driven systems and stable Participant IDs.
6. Prefer data-driven tuning over hard-coded values.
7. Avoid giant Blueprint classes and Blueprint spaghetti.
8. Build debug visibility early.
9. Repository documentation is canonical project memory.

## Future design ideas already discussed
Later versions may include NPC grudges, ego/fear/revenge, visible bandages, banana healing + peel hazards, rotten egg visual/disgust effects, rural cow-dung hazards, urban dog-poop hazards, map-dependent comedy, NPC-to-NPC conflicts, and a comedy pacing director. These are intentionally deferred.

## Hardware planning
Development machine: RTX 3060 with 12 GB dedicated VRAM, 16 GB system RAM, NVMe SSD plus HDD. Keep the active Unreal project on NVMe. Keep first map and traffic/AI counts modest. No hardware purchase is required to start.

## Next engineering task
Create the Unreal Engine project foundation, source-control rules, folder/class structure, then get one controllable motorcycle working before adding other systems.

## New-chat protocol
A new chat should inspect this repository first, read this file, then inspect current code/commits before giving implementation advice. Repository state overrides old chat history.
