# Roadside Idiots — Chat Handoff

Read this first in a new development chat, then inspect `dev/mvp-foundation` and `docs/NEXT_MILESTONE.md` before changing code.

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
- 12 m oval course with one continuous authoritative collision floor
- invisible road-bump bug fixed and repeatedly re-verified
- real 3-2-1-GO input lock and three-lap race
- checkpoints, time, place and circular top-right minimap
- Condition/damage + visible bandage stages
- LEECH / HOTHEAD / PETTY rival personalities and grudges
- banana pickup/heal/peel loop
- rotten egg pickup/throw/stink/grudge loop
- dog/cow poop hazards
- three civilian traffic vehicles
- bots collect/use the same peel and rotten-egg actions as the player
- anti-bunching AI spacing/braking
- comic impact bursts and crash/dizzy camera response

## Imported local visuals
Developer machine has UE Third Person Manny, Fab `MotoInteractionAnims`, `SM_Bike`, riding/mounted/punch/get-hit/dizzy/interaction animations.

Presentation architecture:
- hidden cube chassis remains authoritative physics
- motorcycle + Manny are presentation-only meshes
- final character/vehicle art remains deferred

## Road / camera
- analytic route: 40-point ellipse, radii 9000 cm × 5000 cm
- road width: 1200 cm
- barrier height: ~120 cm
- continuous flat collision floor remains authoritative
- segmented presentation road pieces have collision disabled
- chase camera base arm ~550, height ~185, pitch ~-12.5
- speed-sensitive FOV smoothly widens ~92 -> ~101 degrees

## Rival AI / optimization
- BOT_01 LEECH: pursuit/grudge focused
- BOT_02 HOTHEAD: aggressive/egg focused
- BOT_03 PETTY: peel oriented
- steering/control loop ~= 20 Hz
- expensive awareness scans ~= 5 Hz and staggered
- item decisions separately throttled
- stuck state is per controller
- AI sensing/stuck logic pauses during countdown
- direct rider blockage causes spacing steer + slowing/braking

## Items / hazards
### Banana
- eight pickups
- heals up to 12 Condition and grants peel
- max 3 peels
- F drops a gravity-driven peel
- short owner immunity prevents instant self-hit
- owner can still circle back and slip later

### Rotten egg
- max 2 per bike
- human and AI share the same throw path
- SPLAT + wobble + 1 Condition + stink + grudge attribution
- repeated egg hits refresh one stink actor rather than stacking many

### Dog/cow poop
- map seeds 3 dog piles + 3 cow patties
- dog: short sideways skid/wobble + shorter filth
- cow: speed cut to roughly 42% + longer filth
- one poop mess actor max per bike; repeated hits refresh/upgrade
- stink/splat visuals were reduced so riders remain readable

## Civilian traffic
- yellow SUNDAY DRIVER ~42 km/h
- blue TAXI ~58 km/h with slight wander
- orange DELIVERY VAN ~72 km/h
- overlap-impact architecture avoids hard kinematic deadlocks
- pre-GO racer/traffic contact ignored

## Proven presentation milestones
### VPR-14 / 14.1
- minimap, LAP/POS/time and three-lap flow proven
- HUD cleanup, anti-bunching and DIZZY/camera wobble proven

### VPR-15
- comic WHACK/impact presentation proven
- compact HUD/minimap remains readable
- FILTH status integrated into HUD
- reduced stink no longer hides rider

### VPR-16 / 16.1
- asphalt/lane/barrier/green-roadside/start-finish presentation works without changing collision
- instanced environment reduced PIE actor count from ~542 to ~172
- camera FOV presentation remained stable

### VPR-17 — PASSED 2026-08-15
Local build result:
- UE 5.8 Unity Build succeeded
- actor count remained ~173 after the small polish root
- three-lap finish screen/minimap/race flow still work
- user judged the overall result "not that bad for now"

VPR-17 track polish:
- collision-disabled asphalt join patches reduce green road-corner wedges
- dark inset barrier caps reduce broad yellow slabs
- sparse instanced curve markers

VPR-17 audio:
`RIAudioEvents` is asset-first and checks `/Game/Audio/SFX/SFX_<Event>.SFX_<Event>`.
When no asset exists, it creates short `USoundWaveProcedural` PCM fallback cues.

Fallback event families include:
- Countdown / RaceGo / LapComplete / Finish
- SlapHit / Crash / TrafficHit / Honk
- EggThrow / EggSplat / EggMiss
- PeelSlip
- DogPoop / CowPoop
- PickupBanana / PickupEgg

Distant fallback events receive simple player-distance volume reduction. Real imported SFX override fallbacks automatically later.

Known build note:
- UE 5.8 currently prints C4996 for `USoundWave::GetAssetRegistryTags` from its own `SoundWaveProcedural.h` while compiling our procedural fallback include.
- this is a warning, not a RoadsideIdiots build failure; local build succeeded.

## CURRENT ACTIVE GATE — VPR-18
VPR-18 does not change bike physics, collision, AI logic, race rules or damage tuning.

Changes:
- `URIPresentationWorldSubsystem` caches the human bike instead of scanning all bike actors every presentation tick
- movement component exposes read-only throttle/steering/brake values for presentation only
- generated `EnginePulse` sound follows player speed/throttle
- generated `TireSkid` cue triggers on hard steering/braking at speed with cooldown
- only the human bike receives the continuous prototype engine layer
- banana pickup uses two rotated yellow visual segments rather than one flattened sphere
- dropped banana peel keeps the same proven physics/trigger but uses three thin visual lobes

Local gate:
1. close Unreal, pull latest `dev/mvp-foundation`, compile
2. project must still build under UE 5.8 Unity Build
3. the engine-header C4996 warning may remain
4. actor count should remain in the low-170s plus transient gameplay actors
5. countdown/GO and older generated cues must still work
6. after GO, player should hear a low prototype engine rhythm whose rate/pitch rises with speed
7. hard turn/brake at speed should produce a short skid cue without constant spam
8. banana pickup should look more banana-like
9. F-dropped peel should look three-lobed and still fall/trigger exactly as before
10. no invisible road bumps/jumps may return
11. minimap, 3 laps, AI items, traffic, Condition, slap, egg and recovery must remain stable

## After VPR-18 passes
Prioritize identity/art rather than feature count:
1. improve egg/poop/hazard visuals
2. choose first real environment/map theme and references
3. replace primitive roadside/traffic art gradually while preserving instancing
4. import legally usable real SFX to override generated placeholders
5. revisit bike/rider animation polish after environment direction is clearer
6. multiplayer networking remains deferred until solo presentation is stronger

## Known limitations still deferred
- bot corner/off-track recovery remains imperfect
- motorcycle physics remain prototype physics
- final sounds/map/traffic/item/environment art not implemented
- many visuals still use engine primitives
- sophisticated final traffic physics deferred
- multiplayer networking deferred

## New-chat protocol
1. Read this file.
2. Read `docs/NEXT_MILESTONE.md`.
3. Inspect current `dev/mvp-foundation` head and recent commits.
4. Treat GitHub as more current than old chat text.
5. Continue from the active local gate instead of rebuilding old milestones.
