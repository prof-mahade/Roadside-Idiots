# Roadside Idiots — Chat Handoff

Read this first in a new development chat, then inspect `dev/mvp-foundation` before changing code.

## Project
Roadside Idiots is a Windows PC motorcycle racing game with believable road motion and deliberately funny, petty rider behavior. Long term it is mainly multiplayer; current work is a solo playable prototype.

Tagline: **The road is dangerous. The riders are worse.**

## Branch / local environment
- stable milestone branch: `main`
- active development branch: `dev/mvp-foundation`
- local clone: `C:\GameDev\Roadside-Idiots`
- Unreal Engine 5.8.1
- Visual Studio Community 2026, Game Development with C++
- imported local binary assets are intentionally not stored in Git

## Proven playable foundation
- one player + three motorcycle bots
- W accelerate, S brake/reverse, A/D steer
- Q/E slap, F banana peel, G rotten egg, R recovery, Enter restart
- assisted balance/lean and lateral grip
- 12 m oval prototype course with one continuous authoritative collision floor
- random invisible road-bump bug fixed and repeatedly re-verified
- three-lap race with real 3-2-1-GO control lock, checkpoints, running time and place
- circular top-right minimap with player/rivals/traffic markers
- Condition/damage system with visible bandage stages
- LEECH / HOTHEAD / PETTY rival personalities and grudges
- banana pickup/heal/peel loop
- rotten egg pickup/throw/stink/grudge loop
- dog/cow poop hazards
- three civilian traffic vehicles
- AI item parity: bots can collect/use peels and rotten eggs
- anti-bunching AI spacing/braking
- comic impact text/bursts and crash/dizzy camera response

## Imported local visuals
Developer machine currently has UE Third Person Manny, Fab `MotoInteractionAnims`, `SM_Bike`, riding/mounted/punch/get-hit/dizzy/interaction animations.

Presentation architecture:
- hidden cube chassis remains authoritative physics
- motorcycle + Manny are presentation-only meshes
- rider neutral pose uses the mounted/riding animation setup already proven locally
- final character/vehicle art is still deferred

## Road / camera
- analytic route: 40-point ellipse, radii 9000 cm × 5000 cm
- road width: 1200 cm
- barrier height: ~120 cm
- one continuous flat collision floor is authoritative
- original segmented visual road boxes have collision disabled
- chase camera base arm ~550, height ~185, pitch ~-12.5
- speed-sensitive presentation FOV smoothly widens from roughly 92 degrees at low speed to roughly 101 near 100 km/h
- impact/dizzy camera wobble is separate from speed-FOV logic

## Condition / damage
- side slap damage: 4
- recipient impact immunity: ~0.65 s
- Q/E cooldown: ~0.70 s
- hard collision threshold prevents ordinary scrapes counting as damage
- tip/crash penalty: 3
- banana heals up to 12 Condition
- poop intentionally does not directly change Condition
- countdown damage grace prevents unfair pre-GO traffic damage
- old UE `GEngine not initialized / GetSimplePhysicalMaterial` errors were fixed by moving mass overrides into BeginPlay

## Rival AI
- BOT_01 LEECH: long grudge, pursuit-focused
- BOT_02 HOTHEAD: fast/aggressive, strongest egg pressure
- BOT_03 PETTY: peel-oriented

Shared item architecture:
- every bike owns peel and rotten-egg inventory
- human and AI call the same `DropBananaPeel()` / `ThrowRottenEggAt()` paths
- bots seek useful nearby pickups
- bots avoid traffic, poop, peels and non-target bikes
- HOTHEAD is deliberately more reckless during grudges

Optimization architecture:
- steering/control loop ~= 20 Hz
- expensive world-awareness scans ~= 5 Hz, staggered by bot
- item decisions throttled separately
- stuck time is per AI controller
- AI sensing/stuck logic is suspended during countdown
- bots slow/brake when another bike directly blocks their lane

## Items / hazards
### Banana
- eight pickups
- heals and grants peel
- max 3
- F drops gravity-driven peel
- short self-immunity prevents instant self-hit
- after immunity, even the owner can slip on their own peel
- comic slip/own-goal feedback

### Rotten egg
- max 2 per bike
- G throws player egg; AI uses the same shared throw path
- SPLAT + wobble + 1 Condition damage + stink + grudge attribution
- repeated egg hits refresh one stink actor instead of stacking many

### Dog/cow poop
Map seeds 3 dog piles + 3 cow patties.
- dog: quick sideways skid/wobble + shorter filth/stink
- cow: horizontal speed cut to roughly 42% + longer filth/stink
- at most one poop mess actor per bike; repeated hits refresh/upgrade it
- fumes were reduced from giant balls to narrower rising wisps
- rider splats/glow are intentionally restrained so the motorcycle remains visible

## Civilian traffic
Traffic follows the analytic oval:
- yellow SUNDAY DRIVER ~42 km/h
- blue TAXI ~58 km/h with slight wander
- orange DELIVERY VAN ~72 km/h
- overlap-impact architecture avoids hard kinematic deadlocks
- pre-GO traffic contact is ignored for racers

## Passed presentation gates
### VPR-14 / 14.1
- circular minimap works
- LAP/POS/time strip works
- three-lap flow works
- HUD became compact/readable
- pack spacing improved
- DIZZY + camera wobble retained

### VPR-15
User screenshots proved:
- comic WHACK burst/edge treatment works
- compact HUD/minimap remain readable during combat
- FILTH status integrates into the left panel
- reduced stink no longer hides the rider

### VPR-16
- dark/gray asphalt skin, lane markings, barrier skin, start/finish gantry and roadside props all run without changing collision
- speed-sensitive camera FOV works alongside existing gameplay

### VPR-16.1
User screenshots on 2026-08-15 proved:
- instanced environment version runs
- World Outliner dropped from roughly 542 actors to roughly 172 actors
- green roadside, lane separators, trees/signs and 3-lap/minimap systems remain intact
- core optimization goal passed

## Presentation architecture after VPR-16.1
`URITrackPresentationSubsystem` owns one root actor plus instanced mesh groups for repeated road/environment visuals rather than hundreds of actors.

Repeated presentation pieces are collision-disabled. Collision remains the original world-builder road/barriers.

A small `URITrackPolishSubsystem` added in VPR-17 is also visual-only and uses instanced components to:
- cover small green triangular gaps at visual road-segment joins
- darken the middle of yellow barrier caps, leaving thinner yellow safety trim
- add sparse curve/chevron speed-reference boards

## Audio architecture — VPR-17
`RIAudioEvents` is asset-first:
- it looks for `/Game/Audio/SFX/SFX_<Event>.SFX_<Event>`
- imported real SFX automatically override prototype fallbacks

VPR-17 adds generated `USoundWaveProcedural` PCM fallback cues when an asset is absent, so the prototype is no longer intentionally silent.

Generated fallback families include:
- Countdown / RaceGo
- LapComplete / Finish
- SlapHit / Crash / TrafficHit
- Honk
- EggThrow / EggSplat / EggMiss
- PeelSlip
- DogPoop / CowPoop
- PickupBanana / PickupEgg

Fallback events use simple distance-volume reduction relative to the player so distant AI actions are quieter. These sounds are temporary placeholders, not final audio assets.

## Current active gate — VPR-17
Latest branch contains:
- VPR-16.1 instanced environment (already visually passed)
- track-join/barrier polish subsystem
- audible generated fallback SFX
- cleaner pickup feedback through shared audio events

Local verification required:
1. close Unreal, pull latest `dev/mvp-foundation`, compile `RoadsideIdiotsEditor`
2. Unity Build must compile cleanly
3. PIE actor count should remain close to ~172; a tiny increase is expected
4. green triangular road gaps should be gone/materially reduced
5. barrier caps should read as dark center + thin yellow rim
6. no new road bump/jump may appear; VPR-17 polish is non-colliding
7. countdown and GO should now be audible even without imported SFX
8. banana/egg pickup should each make a distinct cue
9. slap, peel, egg, poop, honk/traffic and crash should make simple generated prototype sounds
10. distant AI cues should be quieter than nearby events
11. minimap, three laps, AI items, traffic, Condition, recovery and camera FOV must remain unchanged

## After VPR-17 passes
Do not add more graybox systems just for feature count. Priorities:
1. lightweight motorcycle engine/tire audio tied to speed/throttle
2. gradually replace generated one-shot audio with legally usable imported SFX
3. improve item/hazard meshes and particles
4. choose first real environment/map art direction and start replacing primitive roadside art
5. preserve current gameplay/race architecture while visual/audio quality increases
6. multiplayer networking remains deferred until solo presentation is stronger

## Known limitations still deferred
- bot corner/off-track recovery remains imperfect
- motorcycle physics are prototype physics, not final two-wheel simulation
- final sounds, map art, traffic models, item models and environment assets are not implemented
- many visuals still use basic engine shapes
- sophisticated final traffic physics is deferred
- multiplayer networking is deferred

## New-chat protocol
1. Read this file.
2. Read `docs/NEXT_MILESTONE.md`.
3. Inspect current `dev/mvp-foundation` head and recent commits.
4. Treat GitHub as more current than old chat text.
5. Continue from the active local gate rather than rebuilding old milestones.
