# Next milestone — VPR-18 Vehicle Sound + Item Readability Gate

VPR-17 passed on the user's machine on 2026-08-15.

## Proven VPR-17 result
- UE 5.8 Unity Build succeeded
- PIE actor count remained ~173, essentially unchanged from the optimized ~172 VPR-16.1 build
- three-lap race, minimap and finish flow still work
- track/environment polish is stable enough for the prototype
- generated fallback SFX are acceptable enough to keep as temporary placeholders
- compiler emits a C4996 deprecation warning from UE 5.8's own `SoundWaveProcedural.h`, but the project builds successfully

## VPR-18 goals
Add lightweight motorcycle/tire sound feel and make banana gameplay objects easier to recognize, without changing proven physics, collision, race, AI or damage behavior.

## VPR-18 changes

### Presentation optimization
`URIPresentationWorldSubsystem` now caches the human motorcycle after first lookup instead of scanning all bike actors every presentation tick.

### Lightweight motorcycle sound
The existing asset-first `RIAudioEvents` path gains two additional events:
- `EnginePulse`
- `TireSkid`

If real `/Game/Audio/SFX/SFX_EnginePulse` or `SFX_TireSkid` assets exist later, they override the generated prototype cues automatically.

For the current asset-free prototype:
- engine pulse rate/pitch rises with motorcycle speed
- throttle slightly increases engine intensity/pitch
- hard steering at speed can trigger a short tire-skid cue
- hard braking at speed can trigger the same tire-skid family
- skid cues are throttled so they do not spam every frame
- only the human motorcycle gets the continuous prototype engine layer, avoiding unnecessary AI audio cost/noise

The movement component exposes read-only throttle/steering/brake getters for presentation systems. No movement tuning or physics equations were changed.

### Banana pickup readability
The banana pickup is no longer one flattened sphere. It now uses two rotated yellow segments to create a curved banana-like silhouette and a slightly reduced glow.

### Dropped peel readability
The dropped peel keeps its proven physics body/trigger but now has three thin visible lobes arranged around the center, making it read more like a peel on the road rather than a yellow oval.

No peel collision radius, gravity, self-immunity, slip impulse or damage logic changed.

## VPR-18 local verification gate
After pulling/compiling latest `dev/mvp-foundation`:
1. UE 5.8 Unity Build must succeed
2. the existing `SoundWaveProcedural.h` C4996 warning may remain; it is not a build failure
3. actor count should remain approximately in the low-170s aside from transient gameplay actors
4. countdown/GO and other VPR-17 generated event sounds must still work
5. after GO, the player motorcycle should have an audible low prototype engine rhythm
6. engine rhythm/pitch should become faster/higher as speed rises, without becoming painfully loud
7. make a hard turn above ~40 km/h or brake hard above ~30 km/h; a short skid/noise cue should occur, but not continuously spam
8. banana pickup should look more curved/distinct than the old glowing oval
9. press F after collecting a peel; the dropped peel should visibly have a three-lobe peel shape and still fall to the road normally
10. run over a peel and verify slip behavior is unchanged
11. confirm no road bumps/jumps returned
12. reconfirm minimap, three laps, AI items, traffic, Condition, Q/E slap, G egg and R recovery

## If VPR-18 passes
Next work should move toward recognizable game art rather than more mechanics:
1. improve egg/poop/hazard presentation using the same lightweight approach
2. choose the first real environment theme/reference direction
3. begin replacing primitive trees/signs/traffic with reusable environment assets while retaining instancing
4. begin importing legally usable real SFX to override generated placeholders
5. revisit rider/bike animation polish once the environment identity is clearer

## Still deferred
- final environment/models/textures
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- multiplayer networking
