# Roadside Idiots — Environment & World Identity Guide

## Goal

The game should eventually be recognizable from a screenshot even when the HUD and title are hidden.

The target is **South-Asian/Bangladesh-inspired roadside arcade reality**, not:
- a generic tropical racetrack,
- a sterile motorsport circuit,
- a misery caricature,
- a photoreal traffic simulator.

The world should feel familiar enough to recognize and exaggerated enough to race through.

---

## Tone

Use affection, specificity and playful exaggeration.

Good humor:
- a badly parked vehicle creates an absurd racing line,
- a roadside stall sits exactly where traffic becomes awkward,
- a cow hazard becomes a tactical problem,
- signs/utility poles/roadside clutter create recognizable visual rhythm,
- rivals behave pettily inside believable mixed traffic.

Bad humor:
- treating poverty itself as a joke,
- depicting local people as inherently stupid,
- using slum imagery as exotic decoration,
- making dangerous real-world road injuries the punchline.

The **riders** are the idiots. The country/culture is not.

---

## Real-world inspiration translated into game language

Bangladesh/South Asian roads commonly involve a heterogeneous mix of road users and vehicle types. Useful game categories include:
- motorcycles,
- CNG/auto-rickshaw-inspired three-wheelers,
- cycle/electric rickshaw-inspired silhouettes,
- compact cars,
- minibuses/buses,
- mini trucks / delivery vehicles,
- larger trucks,
- bicycles,
- pedestrians in safe presentation zones,
- occasional livestock/agricultural-roadside hazards.

For Demo-era gameplay, not every real category needs full simulation. The value comes from **silhouette variety and readable speed/behavior differences**.

---

## World composition

### 1. Road first
The road must remain readable at racing speed.

Do not let visual authenticity destroy playability.

Rules:
- strong road-edge contrast,
- hazards visible early enough to react,
- signage/props outside the main decision corridor unless intentionally interactive,
- avoid foliage covering upcoming traffic,
- utility wires should create atmosphere without visually slicing through the camera at eye level constantly.

### 2. Density rhythm
Avoid uniform decoration around the entire oval/route.

Use alternating zones:

`open green stretch -> small roadside cluster -> busy village/town strip -> open stretch -> service/market cluster -> quieter edge`

This gives the player landmarks and makes one lap easier to mentally learn.

### 3. Landmark memory
Every meaningful route section should eventually have one memorable cue:
- colorful tea stall,
- large roadside tree,
- bus stop/shelter,
- utility transformer/pole cluster,
- market awning,
- construction stack,
- unusual signboard,
- pond/field edge,
- bridge/culvert,
- livestock area.

A player should be able to say “the bend after the tea stall” rather than “turn number four.”

---

## Roadside architecture kit — custom/free only

Build reusable low-cost pieces rather than relying on paid packs.

### Tea stall / small shop
Modular pieces:
- concrete/brick base box,
- corrugated/tin-style roof plane,
- shaded front awning,
- bench,
- counter,
- tea kettle/cups later if a free/custom prop exists,
- colorful sign panel.

### Small residence/shopfront
- simple plaster/concrete shell,
- recessed door/window color blocks,
- tin/flat roof variants,
- wall stains/poster areas later through lightweight materials.

### Road infrastructure
- concrete utility pole,
- simple crossbar,
- restrained overhead cable splines/lines,
- roadside bollards/marker posts,
- drainage edge/culvert suggestion,
- simple guard/bridge pieces where needed.

### Market/service clutter
- stacked crates,
- sacks/boxes,
- benches,
- tarpaulin-style colored roof planes,
- barrels/containers,
- parked noninteractive vehicle silhouettes.

All created or imported content must satisfy the permanent free/custom-only rule.

---

## Vegetation

Current approved free vegetation gives a useful base:
- banana plants,
- tropical ground plants.

Use them in **clusters**, not equal random spacing.

Recommended composition:
- banana clump + low ground plants near homes/stalls,
- isolated broad-canopy tree as landmark,
- sparse roadside weeds near open sections,
- denser planting away from the road where it will not hide hazards.

Avoid making every roadside section look like a plantation simply because banana meshes are available.

---

## Traffic identity roadmap

### Demo 1
Keep current simple civilian vehicle logic because it is functionally accepted.

Presentation can improve without changing traffic AI:
- stronger silhouettes,
- clearer front/rear colors/lights,
- speed-class variation later,
- distinctive horn/audio cues.

### Demo 2 target classes
Prefer a small number of readable behavior archetypes over many cosmetic vehicles:

1. **CNG / three-wheeler inspired**
   - narrow,
   - slower,
   - easy to pass but can occupy awkward lateral position.

2. **Local compact/van**
   - medium speed,
   - baseline predictable traffic.

3. **Bus/minibus inspired**
   - large silhouette,
   - slower response,
   - strong visual obstruction,
   - memorable passing challenge.

4. **Small truck**
   - moderate speed,
   - wide/long obstacle,
   - creates drafting/passing decisions.

Do not add pedestrians directly into high-speed collision gameplay merely for authenticity. Human road users can exist safely in roadside presentation zones until their behavior can be handled responsibly/readably.

---

## Color / visual language

The current prototype uses high-contrast yellow road edging and blue/dark barriers. Keep readability, but future materials should soften the “test track” look.

Direction:
- asphalt/gray road remains neutral,
- safety/edge cues remain high contrast,
- environment uses warmer earth/concrete/tin/paint colors,
- vegetation gives saturated green anchors,
- shops/signs can provide localized strong color,
- rivals/items retain the strongest gameplay colors.

Gameplay markers should win visual competition against scenery.

---

## Soundscape direction

Future free/custom ambient layers can strengthen identity without affecting gameplay simulation:
- distant traffic bed,
- restrained horns,
- birds/insects in quieter sections,
- roadside market/tea-stall murmur only if an appropriately licensed free source is found,
- wind/engine remains dominant at racing speed.

Do not create a constant horn wall. Audio must still communicate impacts/items/engine/skid.

---

## Performance rules

The target user may not have a high-end gaming PC.

Prefer:
- instanced/reused static meshes,
- simple materials,
- modular structures,
- clustered vegetation with sensible draw distances,
- low-complexity collision on scenery,
- non-colliding decorative geometry where interaction is unnecessary.

Avoid spending performance on distant detail the player cannot read at racing speed.

---

## Research/reference basis

Useful factual context:
- World Bank road-safety work on Bangladesh describes roads with pedestrians, animals, bicycles, rickshaws, motorcycles, motorized three-wheelers, cars, minibuses, buses, mini trucks, trucks and agricultural vehicles sharing constrained corridors.
- World Bank reporting has also highlighted the rapid growth and large registered share of motorcycles in Bangladesh.

These sources justify **mixed-road-user diversity as an identity reference**. They do not dictate exact game traffic behavior or imply that unsafe real roads should be reproduced literally.

---

## Screenshot test

Before approving a future environment pass, hide the HUD and ask:

1. Does this look like Roadside Idiots rather than a generic Unreal test track?
2. Can I still read the road at full speed?
3. Can I identify a landmark for this section?
4. Is the cultural inspiration specific but respectful?
5. Did we use only free/custom assets?
6. Does the scenery create future gameplay possibilities without blocking current play?

If the answer to #1 is no, adding more random props is not the solution—improve the identity and composition.
