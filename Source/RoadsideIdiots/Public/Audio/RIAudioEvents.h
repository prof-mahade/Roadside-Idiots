#pragma once

#include "CoreMinimal.h"

class UObject;

namespace RIAudioEvents
{
    // Asset-independent gameplay audio hook. Each event resolves a matching
    // /Game/Audio/SFX/SFX_<Event>.SFX_<Event> asset when one is available.
    // Missing prototype assets are intentionally silent so gameplay never
    // depends on local binary content existing in Git.
    void Play(UObject* WorldContext, FName EventName, const FVector& WorldLocation, float Volume = 1.0f, float Pitch = 1.0f);
}
