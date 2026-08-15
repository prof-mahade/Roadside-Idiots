# Next milestone — VPR-15 Presentation + Audio Hooks Gate

VPR-14.1 is visually passed from the user's latest screenshot. The compact HUD/minimap layout is readable, stink no longer hides the motorcycle, pack spacing is improved, and the multi-lap/minimap architecture remains intact.

## Immediate goal
Validate the first presentation-foundation batch without changing core race mechanics: cleaner world labels, lighter stink visuals, stronger comic impact framing, non-stacking egg stink, and an asset-independent audio event layer.

## VPR-15 changes

### HUD / comic feedback
- build marker: `VPR-15 | PRESENTATION + AUDIO HOOKS`
- player poop/egg status moves into the left HUD instead of floating `COW STINK` / `DOG STINK` / `STINK` labels over the road
- rival labels have shorter range and are suppressed when their projected position overlaps the left HUD, race strip or minimap
- player comic impact text now gets an 8-ray burst and a short red edge vignette
- existing minimap, race strip, finish panel and controls remain

### Filth / stink cleanup
- poop remains one `ARIPoopMessEffect` per bike; repeated hits refresh/upgrade it
- poop uses three narrow rising wisps instead of large round green spheres
- poop glow and attached splats are reduced again
- rotten-egg stink uses three small wisps instead of five large puffs
- repeated egg hits refresh one existing stink actor rather than stacking several clouds

### Optional audio foundation
New `RIAudioEvents` helper resolves optional assets from:

`/Game/Audio/SFX/SFX_<Event>.SFX_<Event>`

Missing local assets are silent and cached for the current editor run, so gameplay never depends on binary sound content being present in Git.

Wired events:
- Countdown
- RaceGo
- LapComplete
- Finish
- SlapHit
- PeelSlip
- EggThrow
- EggSplat
- EggMiss
- DogPoop
- CowPoop
- Honk
- TrafficHit
- Crash

`URIPresentationWorldSubsystem` owns countdown / GO / lap / finish / crash presentation cues without changing `ARIRaceManager` race rules.

Exact optional sound naming is documented in `docs/AUDIO_ASSET_CONVENTION.md`.

## VPR-15 local gate
After pulling/compiling latest `dev/mvp-foundation`:
1. verify `VPR-15 | PRESENTATION + AUDIO HOOKS`
2. hit dog/cow poop; player filth status should be in the left HUD and the green effect should look thinner/more wisp-like
3. hit multiple poop hazards; effects should refresh rather than stack
4. get egged more than once before stink expires; there should still be one compact egg-stink effect
5. slap a rival, hit a peel, throw an egg and contact traffic; mechanics/comic feedback must still work
6. actual sound is expected to remain silent until matching `/Game/Audio/SFX/` assets are imported
7. deliberately tip the bike; `DIZZY!`, camera wobble and recovery must still work
8. confirm rival labels do not draw over the minimap/top strip/left HUD
9. reconfirm countdown, minimap, three laps, F/G item use, AI item use, traffic, Condition and flat road

## If VPR-15 passes
Next batch:
1. import or generate legally usable prototype SFX using the established naming convention
2. add a dedicated motorcycle engine loop with speed/RPM-style pitch rather than treating it as a one-shot event
3. add tire/skid loop and spatial traffic ambience where useful
4. then start environment/map art replacement while keeping the proven gameplay loop intact

## Still deferred
- final environment/models/textures
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- multiplayer networking
