# Next milestone — VPR-19 Roadside Identity Gate

VPR-18 was accepted by the user on 2026-08-15 as **"not bad, playable for now"**. Treat the current mechanics/audio/physics/race stack as a frozen playable baseline unless a real regression appears.

## Proven baseline through VPR-18
- UE 5.8 Unity Build succeeds; the engine-owned `SoundWaveProcedural.h` C4996 warning is non-fatal
- three-lap race, minimap, finish flow, Condition, items, traffic and rival AI remain playable
- continuous flat collision road is still authoritative; no invisible road-bump regression
- instanced road/environment architecture keeps PIE actor count in the low-170s plus transient gameplay actors
- generated fallback one-shot audio is usable for prototype testing
- lightweight player engine pulse + skid cues are acceptable as temporary presentation
- banana pickup/drop visuals are more readable while retaining proven mechanics

## VPR-19 goal
Stop adding graybox mechanics and establish the first recognizable roadside identity while preserving the playable baseline.

## VPR-19 changes

### Reversible roadside theme scaffold
New `URIRoadsideThemeSubsystem` creates one collision-disabled presentation actor with instanced mesh groups. It does not alter road/barrier collision, race logic, AI, damage or motorcycle movement.

The first theme is a **South-Asian/Bangladesh-inspired roadside graybox scaffold**, intentionally generic/reversible until real reference art/assets are chosen.

It adds:
- low-rise roadside stall/tea-shop silhouettes
- small plaster/brick houses with colored tin-style roofs
- a simple open roadside shelter/bus-stop silhouette
- sparse utility poles, cross-arms and visual overhead lines
- field patches and a few water/pond patches beyond the barriers
- compact vegetation clusters behind roadside structures
- dirt shoulder/plaza patches under landmark clusters

All theme geometry is collision-disabled and positioned outside the 12 m racing surface.

### Hazard/item readability
- dog poop is now a small stacked pile silhouette
- cow poop remains broad/flat, making the two road hazards visually different before impact
- trigger sizes and gameplay effects are unchanged
- rotten-egg pickup now slowly rotates for better item readability at speed; inventory behavior is unchanged

## VPR-19 local verification gate
After pulling/compiling latest `dev/mvp-foundation`:
1. UE 5.8 Unity Build must succeed
2. actor count should rise only slightly from the low-170s because the theme uses one root actor + instanced components
3. no roadside theme object may intrude onto the racing surface or create collision/bump/jump behavior
4. stalls/houses/utility poles/fields should make the course feel less empty without making it visually busy
5. road/minimap/three-lap race/traffic/AI/items/Condition/recovery remain stable
6. dog and cow poop should be visually distinguishable before hitting them
7. rotten-egg pickup should visibly rotate and still collect normally
8. VPR-18 engine/skid and VPR-17 one-shot audio should remain intact

## If VPR-19 passes
Next work should use references/assets rather than more primitive-code scenery:
1. choose/confirm the first real map art direction with the user
2. research legally usable Fab/environment packs and real SFX that fit that direction
3. replace the reversible primitive stalls/trees/signs with reusable art assets while preserving instancing
4. improve bike/rider animation transitions and presentation only after the environment direction is clear
5. keep multiplayer networking deferred until solo presentation is materially stronger

## Still deferred
- final environment/models/textures
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- multiplayer networking
