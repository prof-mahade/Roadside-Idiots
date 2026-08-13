# Roadside Idiots — Architecture

Canonical engineering rules:

1. Keep bike, rider, race, AI, traffic, UI, audio/VFX, and debug systems separate.
2. Gameplay state and presentation must not be mixed.
3. Every racer gets a stable Participant ID for the whole match.
4. AI strategy decides goals; AI driving handles movement.
5. Prefer event-driven communication between systems.
6. Prefer data assets/tables for tunable values.
7. Avoid giant Blueprints; use small reusable components.
8. Only physics and truly frame-dependent logic should tick every frame.
9. Solo MVP first, but do not assume the local client will own all future authoritative state.
10. Repository docs are the source of truth for future chats.

Recommended Unreal project/module name: `RoadsideIdiots`.
Recommended prefix: `RI`.

Initial code areas: Core, Vehicle, Rider, Race, Interaction, AI, Traffic, Debug.
Initial content areas: Vehicles, Riders, AI, Interactions, Race, Traffic, UI, Audio, VFX, Data, Maps, Tests.

Generated Unreal folders such as Binaries, DerivedDataCache, Intermediate, and Saved should not be committed.
