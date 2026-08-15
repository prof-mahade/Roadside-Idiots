#pragma once

#include "CoreMinimal.h"

class UObject;

namespace RIAudioEvents
{
    // Asset-first gameplay audio hook. Each event resolves a matching
    // /Game/Audio/SFX/SFX_<Event>.SFX_<Event> asset when one is available.
    // If the asset is missing, a tiny generated prototype cue is used instead
    // so the playable build still has immediate audio feedback. Imported SFX
    // therefore replace the fallback automatically without gameplay refactors.
    void Play(UObject* WorldContext, FName EventName, const FVector& WorldLocation, float Volume = 1.0f, float Pitch = 1.0f);
}
