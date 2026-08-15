# Next milestone — VPR-16.1 Instanced Track + Environment Cleanup Gate

VPR-16 visually passed from the user's screenshots: the road skin, lane markings, barrier caps, start/finish gantry, minimap, three-lap race and camera presentation are all running. The screenshots also exposed two issues worth fixing before adding more world art: roadside ground still read too dark/empty, and the first presentation implementation increased PIE actor count from roughly 171 to roughly 542 because repeated road/scenery pieces were spawned as individual actors.

## VPR-16.1 goals
Improve environment readability and reduce presentation overhead without touching the proven collision road, race rules, AI, items, damage, traffic or motorcycle movement.

## VPR-16.1 changes
### Instanced presentation architecture
`URITrackPresentationSubsystem` now owns one presentation root actor and groups repeated geometry into `UInstancedStaticMeshComponent`s by mesh/material color.

Repeated pieces no longer require one actor each:
- asphalt segments
- lane markings
- yellow edge/cap strips
- barrier shells
- grass/verge pieces
- tree trunks/leaves
- sign boards/posts
- start/finish tiles and gantry pieces

Expected result: PIE actor count should fall dramatically from the ~542 seen in VPR-16, while preserving the same or richer scenery.

All presentation components remain collision-disabled. The original continuous collision floor and original barrier collision remain authoritative.

### Environment cleanup
- brighter green base ground
- darker-green verge bands outside both barriers
- three broad road lanes using two dashed separators
- cleaner asphalt/concrete/yellow palette
- more roadside trees using instances
- eight colored landmark boards instead of four
- small roadside reflector posts
- refined checkered start/finish strip
- more deliberate gantry with alternating overhead blocks and side timing boards

### Camera
Speed-sensitive FOV remains presentation-only:
- ~92 degrees low speed
- ~101 degrees near 100 km/h
- smooth interpolation

## Local verification gate
After pulling/compiling latest `dev/mvp-foundation`:
1. compile must succeed under Unreal Unity Build
2. drive one normal lap and verify there are still no road seam bumps/jumps
3. roadside should read green rather than a black void
4. road should show two dashed lane separators plus yellow edges
5. start/finish gantry should look more deliberate than VPR-16
6. trees/signs/posts must remain outside the racing surface
7. collision against barriers must behave exactly as before
8. minimap, three laps, AI, traffic, F/G items, Q/E slap, Condition and recovery must still work
9. check World Outliner actor count during PIE; it should be far below the ~542 actor VPR-16 screenshot because track/scenery repetition is now instanced
10. verify high-speed FOV still widens smoothly

## If VPR-16.1 passes
Next work should focus on actual game feel/assets rather than more graybox systems:
1. real/prototype motorcycle engine loop and tire/skid audio
2. legally usable slap/honk/splat/poop/peel/crash SFX assets using the existing audio-event convention
3. better item/hazard meshes and particles
4. replace primitive environment/traffic models gradually while preserving the established gameplay architecture
5. then revisit final map theme/art direction before multiplayer networking

## Still deferred
- final environment/models/textures
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- multiplayer networking
