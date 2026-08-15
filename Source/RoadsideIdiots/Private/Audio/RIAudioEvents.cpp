#include "Audio/RIAudioEvents.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    TSet<FName> MissingAudioEvents;
}

namespace RIAudioEvents
{
    void Play(UObject* WorldContext, const FName EventName, const FVector& WorldLocation, const float Volume, const float Pitch)
    {
        if (!WorldContext || EventName.IsNone() || MissingAudioEvents.Contains(EventName)) return;

        const FString AssetName = FString::Printf(TEXT("SFX_%s"), *EventName.ToString());
        const FString AssetPath = FString::Printf(TEXT("/Game/Audio/SFX/%s.%s"), *AssetName, *AssetName);
        USoundBase* Sound = LoadObject<USoundBase>(nullptr, *AssetPath);
        if (!Sound)
        {
            MissingAudioEvents.Add(EventName);
            return;
        }

        UGameplayStatics::PlaySoundAtLocation(
            WorldContext,
            Sound,
            WorldLocation,
            FMath::Clamp(Volume, 0.0f, 2.5f),
            FMath::Clamp(Pitch, 0.55f, 1.75f));
    }
}
