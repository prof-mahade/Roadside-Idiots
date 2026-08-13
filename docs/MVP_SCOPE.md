# Roadside Idiots — MVP Scope

## Goal
Prove that the core game is fun before adding advanced features.

## Playable loop
`Start race -> ride -> race AI -> fight -> crash/recover -> finish -> restart`

## Required for first playable
- One compact race route
- One motorcycle type
- One player rider
- Three to four AI riders
- Basic civilian traffic
- Acceleration, braking, steering, assisted lean/balance
- Basic slap/punch and left/right kick
- Rider health and simple damage
- Crash detection
- Rider ragdoll
- Recovery/reset
- Checkpoints and race progress
- Finish detection and restart
- Basic HUD: speed, health, race position/progress
- Simple AI retaliation/aggression
- Basic hit and crash sounds
- A few simple funny reactions

## Explicitly deferred
- Online multiplayer
- Open world
- Character creator
- Shops/garage
- Progression/unlocks
- Large inventory
- Many bikes
- Many weapons
- Police system
- Advanced grudge/relationship network
- Detailed bandage/body injury visuals
- Banana, rotten egg, cow dung, dog poop, oil and other chaos items/hazards
- Weather/day-night
- Story/career mode
- Replay/highlight system
- Advanced NPC personality archetypes

## Definition of done
The MVP is done when:
1. A Windows build launches outside Unreal Editor.
2. The player can complete a full race from start to finish.
3. Three to four AI riders can complete the race.
4. The bike feels controllable and enjoyable.
5. Combat reads clearly and produces reactions.
6. Crashes can happen and the player can recover/reset.
7. AI retaliation creates at least occasional memorable moments.
8. The player can restart immediately.
9. There are no blockers that prevent completing a race.

## Feature gate
Before adding anything new, ask:
- Is it necessary to prove the core loop?
- Does it make the current playable version more fun?
- Will it delay a complete playable race?
- Can it be added later without architectural damage?

If it is not required for the first playable, put it in the backlog.
