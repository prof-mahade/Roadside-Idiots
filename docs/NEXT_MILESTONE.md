# Next milestone — VPR-16 Track Skin + Camera Feel Gate

VPR-15 is visually passed from the user's screenshots. The compact HUD/minimap stayed readable, comic WHACK feedback is visible, filth status is correctly integrated into the HUD, and the reduced stink effect no longer hides the motorcycle.

## Immediate goal
Improve the biggest remaining visual weakness—the empty checkerboard prototype track—without touching the proven collision road, race rules, AI, items, damage, or motorcycle movement.

## VPR-16 changes

### Visual-only track skin
A new `URITrackPresentationSubsystem` adds a non-colliding presentation layer over the existing course:
- dark asphalt overlay over the white checkerboard road
- dashed centre guides for curvature/speed readability
- yellow edge lines
- concrete-colored barrier shells with yellow top caps
- muted green ground outside the road
- visible checkered start/finish strip
- simple start/finish gantry
- sparse stylised roadside trees
- four colored roadside landmark/sign boards

The original 12 m course geometry and its seamless continuous collision floor remain unchanged underneath. All new track/scenery actors have collision disabled.

### Camera feel
The same presentation subsystem finds the human rider's chase camera and smoothly changes FOV with speed:
- ~92 degrees at low speed
- up to ~101 degrees near 100 km/h
- smooth interpolation rather than an instant zoom

This changes presentation only; bike forces, steering, grip and camera impact/dizzy wobble logic remain unchanged.

### Build marker
HUD marker becomes:

`VPR-16 | TRACK SKIN + CAMERA FEEL`

## VPR-16 local gate
After pulling and compiling latest `dev/mvp-foundation`:
1. verify the VPR-16 HUD marker
2. road should now be dark asphalt rather than white checkerboard
3. grass/green ground should be visible outside the barriers
4. center/edge road markings should follow the entire oval cleanly
5. barriers should visually read as concrete with yellow top caps while collision still behaves exactly as before
6. find the checkered start/finish strip and gantry
7. roadside trees/sign boards should give visible speed reference without entering the racing surface
8. accelerate from low speed toward top speed and confirm the camera widens smoothly, not abruptly
9. deliberately drive normally over several old road-segment boundaries; there must be no return of the invisible-bump/jump bug
10. reconfirm minimap, three laps, AI, traffic, F/G items, Q/E slap, Condition, poop/egg effects and recovery

## If VPR-16 passes
Proceed with a combined sound + world-quality batch:
1. prototype engine/tire audio path or locally imported SFX
2. reduce remaining primitive-looking filth/item visuals
3. improve roadside set dressing and lighting
4. start replacing placeholder car/item/environment meshes while retaining the established gameplay architecture

## Still deferred
- final environment/models/textures
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- multiplayer networking
