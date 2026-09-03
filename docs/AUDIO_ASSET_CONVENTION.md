# Roadside Idiots — Prototype Audio Asset Convention

Gameplay audio is intentionally asset-independent. Code fires named events through `RIAudioEvents`; if a matching Unreal `USoundBase` asset exists, it plays. If the asset is missing, gameplay remains silent and continues normally.

## Folder
Import/create prototype sounds under:

`/Game/Audio/SFX/`

## Naming
Each event resolves this exact asset path:

`/Game/Audio/SFX/SFX_<Event>.SFX_<Event>`

Current wired events:

- `SFX_Countdown` — each 3/2/1 countdown tick
- `SFX_RaceGo` — GO cue
- `SFX_LapComplete` — player completes lap 1 or lap 2
- `SFX_Finish` — player completes the final lap
- `SFX_SlapHit` — Q/E side hit connects
- `SFX_PeelSlip` — a bike hits a banana peel
- `SFX_EggThrow` — rotten egg projectile launches
- `SFX_EggSplat` — rotten egg hits a rider
- `SFX_EggMiss` — rotten egg hits the world
- `SFX_DogPoop` — dog-poop skid
- `SFX_CowPoop` — cow-patty splorch/slowdown
- `SFX_Honk` — civilian traffic contact
- `SFX_TrafficHit` — civilian traffic impact layer
- `SFX_Crash` — bike tips/crashes

## Design rules
- Keep clips short and punchy for prototype iteration.
- Avoid copyrighted commercial-game sounds.
- Prefer original, CC0, or appropriately licensed assets.
- Normalize loudness before final balancing; code intentionally keeps event volumes near 1.0.
- Pitch variation is already applied by several gameplay events so repeated hits sound less identical.
- Missing assets are cached as silent for the current editor run; restart PIE/editor after importing a previously missing event asset.

## Later additions
The engine loop, tire/skid loop, ambience, crowd/world audio, UI clicks and music should use dedicated looping/audio-component architecture rather than this one-shot event helper.
