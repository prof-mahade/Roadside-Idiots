# Next milestone — VPR-22 Moving-Object Presentation Gate

VPR-21 passed visually on the user's machine on 2026-08-15.

## Permanent project constraint — FREE ONLY
Roadside Idiots must use only:
- assets/tools/content available to the user for $0 under the applicable license, or
- assets/models/materials/audio we create ourselves.

Do not recommend, plan around, purchase, or retain paid packs as a future dependency. If a suitable free asset does not exist, build a lightweight custom replacement.

## Frozen playable baseline
- VPR-18 gameplay/physics/race/audio baseline remains frozen unless a real regression appears
- VPR-19 roadside-theme foundation remains frozen
- VPR-20/20.1 free vegetation integration is visually accepted
- VPR-21 custom/free roadside-art layer is visually accepted
- three-lap race, minimap, finish flow, Condition, items, hazards, traffic and rival AI remain playable
- continuous flat collision road remains authoritative
- scenery/presentation geometry remains collision-disabled
- instanced presentation architecture remains mandatory

## VPR-21 — PASSED
User screenshots confirmed:
- roadside house facades now show doors/windows/veranda/roof detailing rather than only plain boxes
- shelter/stall details are visible
- free banana/tropical vegetation still renders correctly
- environment remains outside the authoritative race surface
- PIE actor count is 175, exactly matching the expected +1 visual-root change from VPR-20.1
- race/minimap/rivals/traffic were still operating in the screenshots

## VPR-22A — CODED, LOCAL COMPILE/VISUAL GATE PENDING
A new `URITrafficStyleSubsystem` changes only civilian-traffic presentation. It does not alter traffic movement, speed, lane offsets, wandering, collision/overlap volume, damage, honk or impact behavior.

VPR-22A:
- keeps the existing three `ARITrafficVehicle` actors and their authoritative `ImpactVolume`
- restyles the existing body/cabin/wheel components into three visibly different silhouettes
- adds visual-only cube details with `NoCollision` and overlap generation disabled
- gives one vehicle a low compact-sedan shape with glass and bumpers
- gives one vehicle a taxi-like sedan shape with roof sign and side stripes
- gives one vehicle a taller delivery-van shape with cargo box, windshield, rear-door line and bumper
- uses only Engine basic shapes/materials; no new paid or external assets
- does not add any actors, so baseline PIE actor count should remain ~175
- uses uniquely prefixed namespace symbols to avoid Unity-build redefinition regressions

The exact semantic mapping of style to traffic label is intentionally non-authoritative: all three style variants are presentation-only and mechanically equivalent. If TActorIterator ordering differs, the three vehicles may swap visual styles without affecting gameplay.

## VPR-22A local verification gate
1. close Unreal and pull current `dev/mvp-foundation`
2. compile `RoadsideIdiotsEditor Win64 Development`
3. launch PIE and drive until all three traffic vehicles are visible
4. confirm they no longer share the same toy-block silhouette
5. confirm one reads compact/sedan-like, one taxi-like, and one taller/van-like
6. confirm added glass/sign/stripe/cargo details are attached correctly and do not float far from the vehicles
7. confirm there is no new collision, snagging or impact-volume change
8. confirm PIE actor count stays roughly 175 plus transient gameplay actors
9. confirm race/minimap/rivals/items/hazards remain unchanged

HUD may still display `VPR-20.1 | FREE VEG DENSITY + CLEANUP` during this gate. That string is not used as the authoritative milestone state; this document and GitHub head are.

## After VPR-22A passes
Continue VPR-22 with small, visual-only work in this order:
1. rider/bike presentation polish (subtle steering lean / pose alignment only; no chassis retune)
2. item/hazard visual cleanup only where a screenshot shows a readability problem
3. freeze VPR-22 once moving objects are readable enough for Demo 1

Do not reopen proven movement/AI/item mechanics just to chase presentation polish.

## Demo 1 definition
Demo 1 is a packaged Windows solo build; multiplayer is not required.

Required before calling Demo 1 ready:
1. one visually coherent 3-lap course
2. stable player bike + three rivals
3. stable AI item use, traffic, hazards, recovery and finish flow
4. minimap/HUD/countdown/results readable
5. banana/egg/poop/slap comedy loops working
6. usable engine/skid/impact/item/audio feedback
7. free/custom coherent environment dressing
8. simple title/start/restart/quit flow
9. packaged Windows build launches outside the editor
10. final bug/performance/packaging sweep

## Remaining demo milestones
- VPR-22: moving-object presentation — current local gate
- VPR-23: title/menu/pause/settings/restart/quit + Windows packaging flow
- VPR-24: final demo bug/performance/package audit

## Deferred beyond Demo 1
- multiplayer networking
- sophisticated final motorcycle/traffic physics
- perfect off-track recovery
- final commercial-quality map/assets/audio
- additional maps/modes
