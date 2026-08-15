# Next milestone — VPR-17 Audible Prototype + Track Polish Gate

VPR-16.1 is visually passed from the user's 2026-08-15 screenshots.

## Proven VPR-16.1 result
- instanced track/environment presentation compiles and runs
- World Outliner dropped from roughly 542 actors in the first VPR-16 implementation to roughly 172 actors
- road, barriers, green roadside, two dashed lane separators, trees/signs, minimap and three-lap race remain intact
- the original seamless collision road remains authoritative and no collision architecture was changed

The screenshots still exposed a few polish issues: occasional small green triangular gaps at visual road-segment joins, very broad yellow barrier caps, and a prototype that still had silent audio hooks when no local SFX assets existed.

## VPR-17 changes

### Track polish layer
New `URITrackPolishSubsystem` is visual-only and adds one lightweight root actor with instanced components.

It adds:
- dark asphalt join patches at all 40 route vertices to hide small green corner wedges between straight visual road chords
- dark inset strips over the existing yellow barrier caps, leaving yellow as a thinner safety trim instead of a glowing slab
- sparse non-colliding curve/chevron markers for extra speed reference

No road/barrier collision, race rules, AI, items, damage, traffic or motorcycle movement code is changed.

### Audible generated fallback SFX
`RIAudioEvents` remains asset-first. It still checks for `/Game/Audio/SFX/SFX_<Event>` assets first.

When a real SFX asset is missing, VPR-17 now creates a short `USoundWaveProcedural` PCM fallback instead of remaining silent. This gives the playable prototype immediate generated cues while preserving the final asset pipeline.

Generated fallback families include:
- countdown / GO
- lap complete / finish
- slap / crash / traffic impact
- honk
- rotten egg throw / splat / miss
- banana-peel slip
- dog/cow poop
- banana / rotten-egg pickup cues

Fallback cues receive simple distance-volume reduction relative to the human rider so distant AI events are quieter. Imported real SFX assets still override the generated fallback automatically.

### Pickup cleanup
- banana and rotten-egg collection no longer add extra debug-message spam
- human pickup feedback now uses the shared audio-event path
- rotten-egg pickup silhouette is slightly tilted/darker for faster visual distinction

## VPR-17 local verification gate
After pulling/compiling latest `dev/mvp-foundation`:
1. compile must succeed under UE 5.8 Unity Build
2. PIE actor count should remain close to the VPR-16.1 ~172 range; a tiny increase for the polish root is expected
3. drive through several bends and verify the previous green triangular road gaps are gone or materially reduced
4. barrier tops should read as dark caps with a thinner yellow rim rather than broad yellow slabs
5. there must be no new invisible road bump/jump because all VPR-17 track polish geometry is collision-disabled
6. listen for generated countdown / GO cues at race start
7. collect a banana and rotten egg; each should make a distinct short pickup cue
8. trigger slap, peel slip, egg splat, poop, traffic/honk and crash when practical; they should now make prototype generated sounds even with no `/Game/Audio/SFX` assets
9. distant AI-triggered fallback sounds should be quieter than nearby events
10. minimap, three laps, AI item parity, traffic, Condition, recovery and camera FOV must remain unchanged

## If VPR-17 passes
Next batch should move away from graybox mechanics and into stronger identity:
1. add a lightweight motorcycle engine/tire audio layer tied to speed/throttle
2. replace generated one-shot fallbacks gradually with legally usable imported SFX
3. improve banana/egg/poop/peel models and particle presentation
4. choose the first real environment/map theme and begin replacing primitive roadside art
5. retain the established gameplay/race architecture while art quality increases

## Still deferred
- final environment/models/textures
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- multiplayer networking
