#include "Audio/RIAudioEvents.h"

#include "GameFramework/Pawn.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundWaveProcedural.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr uint32 RIFallbackSampleRate = 22050;

    TSet<FName> MissingAudioEvents;

    struct FRIFallbackSoundHandle
    {
        FRIFallbackSoundHandle(USoundWaveProcedural* InWave, const double InExpireAt)
            : Wave(InWave)
            , ExpireAt(InExpireAt)
        {
        }

        TStrongObjectPtr<USoundWaveProcedural> Wave;
        double ExpireAt = 0.0;
    };

    TArray<FRIFallbackSoundHandle> ActiveFallbackSounds;

    void PruneFallbackSounds()
    {
        const double Now = FPlatformTime::Seconds();
        ActiveFallbackSounds.RemoveAll([Now](const FRIFallbackSoundHandle& Handle)
        {
            return Now >= Handle.ExpireAt;
        });
    }

    float NextNoise(uint32& State)
    {
        State = State * 1664525u + 1013904223u;
        const uint32 Bits = (State >> 8u) & 0xFFFFu;
        return static_cast<float>(Bits) / 32767.5f - 1.0f;
    }

    USoundWaveProcedural* BuildFallbackWave(const FName EventName)
    {
        float Duration = 0.18f;
        float BaseFrequency = 520.0f;
        float Amplitude = 0.34f;

        const bool bCountdown = EventName == FName(TEXT("Countdown"));
        const bool bRaceGo = EventName == FName(TEXT("RaceGo"));
        const bool bFinish = EventName == FName(TEXT("Finish"));
        const bool bLapComplete = EventName == FName(TEXT("LapComplete"));
        const bool bHonk = EventName == FName(TEXT("Honk"));
        const bool bCrash = EventName == FName(TEXT("Crash"));
        const bool bSlap = EventName == FName(TEXT("SlapHit"));
        const bool bTrafficImpact = EventName == FName(TEXT("TrafficImpact")) || EventName == FName(TEXT("TrafficHit"));
        const bool bEggThrow = EventName == FName(TEXT("EggThrow"));
        const bool bEggSplat = EventName == FName(TEXT("EggSplat"));
        const bool bEggMiss = EventName == FName(TEXT("EggMiss"));
        const bool bPeelSlip = EventName == FName(TEXT("PeelSlip"));
        const bool bDogPoop = EventName == FName(TEXT("DogPoop"));
        const bool bCowPoop = EventName == FName(TEXT("CowPoop"));
        const bool bPickupBanana = EventName == FName(TEXT("PickupBanana"));
        const bool bPickupEgg = EventName == FName(TEXT("PickupEgg"));
        const bool bEnginePulse = EventName == FName(TEXT("EnginePulse"));
        const bool bTireSkid = EventName == FName(TEXT("TireSkid"));

        if (bCountdown)
        {
            Duration = 0.115f;
            BaseFrequency = 650.0f;
            Amplitude = 0.26f;
        }
        else if (bRaceGo)
        {
            Duration = 0.28f;
            BaseFrequency = 820.0f;
            Amplitude = 0.30f;
        }
        else if (bLapComplete)
        {
            Duration = 0.24f;
            BaseFrequency = 780.0f;
            Amplitude = 0.25f;
        }
        else if (bFinish)
        {
            Duration = 0.42f;
            BaseFrequency = 620.0f;
            Amplitude = 0.28f;
        }
        else if (bHonk)
        {
            Duration = 0.30f;
            BaseFrequency = 360.0f;
            Amplitude = 0.28f;
        }
        else if (bCrash || bTrafficImpact)
        {
            Duration = 0.24f;
            BaseFrequency = 105.0f;
            Amplitude = 0.42f;
        }
        else if (bSlap)
        {
            Duration = 0.12f;
            BaseFrequency = 160.0f;
            Amplitude = 0.38f;
        }
        else if (bEggThrow)
        {
            Duration = 0.20f;
            BaseFrequency = 430.0f;
            Amplitude = 0.22f;
        }
        else if (bEggMiss)
        {
            Duration = 0.11f;
            BaseFrequency = 300.0f;
            Amplitude = 0.16f;
        }
        else if (bEggSplat || bDogPoop || bCowPoop)
        {
            Duration = bCowPoop ? 0.34f : 0.23f;
            BaseFrequency = bCowPoop ? 82.0f : 125.0f;
            Amplitude = bCowPoop ? 0.38f : 0.30f;
        }
        else if (bPeelSlip)
        {
            Duration = 0.24f;
            BaseFrequency = 720.0f;
            Amplitude = 0.24f;
        }
        else if (bPickupBanana || bPickupEgg)
        {
            Duration = 0.16f;
            BaseFrequency = bPickupBanana ? 760.0f : 430.0f;
            Amplitude = 0.20f;
        }
        else if (bEnginePulse)
        {
            Duration = 0.135f;
            BaseFrequency = 92.0f;
            Amplitude = 0.26f;
        }
        else if (bTireSkid)
        {
            Duration = 0.18f;
            BaseFrequency = 980.0f;
            Amplitude = 0.16f;
        }

        const int32 SampleCount = FMath::Max(1, FMath::RoundToInt(Duration * static_cast<float>(RIFallbackSampleRate)));
        TArray<int16> PCM;
        PCM.SetNumUninitialized(SampleCount);

        uint32 NoiseState = GetTypeHash(EventName) ^ 0x9E3779B9u;
        float Phase = 0.0f;

        for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
        {
            const float Time = static_cast<float>(SampleIndex) / static_cast<float>(RIFallbackSampleRate);
            const float Alpha = static_cast<float>(SampleIndex) / static_cast<float>(FMath::Max(1, SampleCount - 1));
            const float Attack = FMath::Clamp(Time / (bEnginePulse ? 0.006f : 0.012f), 0.0f, 1.0f);
            const float ReleasePower = bEnginePulse ? 0.55f : 1.45f;
            const float Release = FMath::Pow(FMath::Clamp(1.0f - Alpha, 0.0f, 1.0f), ReleasePower);
            const float Envelope = Attack * Release;

            float Frequency = BaseFrequency;
            if (bRaceGo || bEggThrow || bPickupBanana)
            {
                Frequency *= FMath::Lerp(0.72f, 1.65f, Alpha);
            }
            else if (bPickupEgg)
            {
                Frequency *= FMath::Lerp(1.10f, 0.72f, Alpha);
            }
            else if (bPeelSlip)
            {
                Frequency *= 1.0f + 0.15f * FMath::Sin(Alpha * PI * 7.0f);
            }
            else if (bEnginePulse)
            {
                Frequency *= 0.96f + 0.05f * FMath::Sin(Alpha * PI * 4.0f);
            }

            Phase += 2.0f * PI * Frequency / static_cast<float>(RIFallbackSampleRate);
            float Value = FMath::Sin(Phase);

            if (bHonk)
            {
                Value = 0.58f * FMath::Sin(Phase) + 0.42f * FMath::Sin(Phase * 1.38f);
            }
            else if (bFinish)
            {
                Value = 0.50f * FMath::Sin(Phase) + 0.32f * FMath::Sin(Phase * 1.25f) + 0.18f * FMath::Sin(Phase * 1.50f);
            }
            else if (bLapComplete)
            {
                Value = 0.68f * FMath::Sin(Phase) + 0.32f * FMath::Sin(Phase * 1.50f);
            }
            else if (bCrash || bTrafficImpact || bSlap)
            {
                const float Noise = NextNoise(NoiseState);
                const float ToneWeight = bSlap ? 0.18f : 0.34f;
                Value = ToneWeight * FMath::Sin(Phase) + (1.0f - ToneWeight) * Noise;
            }
            else if (bEggSplat || bDogPoop || bCowPoop)
            {
                const float Noise = NextNoise(NoiseState);
                const float WetPulse = FMath::Sin(Phase) * (0.55f + 0.45f * FMath::Sin(Alpha * PI * 5.0f));
                Value = 0.52f * WetPulse + 0.48f * Noise;
            }
            else if (bEggMiss)
            {
                Value = 0.65f * FMath::Sin(Phase) + 0.35f * NextNoise(NoiseState);
            }
            else if (bEnginePulse)
            {
                const float Noise = NextNoise(NoiseState);
                Value =
                    0.50f * FMath::Sin(Phase) +
                    0.25f * FMath::Sin(Phase * 2.0f) +
                    0.13f * FMath::Sin(Phase * 3.0f) +
                    0.12f * Noise;
            }
            else if (bTireSkid)
            {
                const float Noise = NextNoise(NoiseState);
                const float Whine = FMath::Sin(Phase) * (0.55f + 0.45f * FMath::Sin(Alpha * PI * 9.0f));
                Value = 0.24f * Whine + 0.76f * Noise;
            }

            const float Scaled = FMath::Clamp(Value * Envelope * Amplitude, -0.96f, 0.96f);
            PCM[SampleIndex] = static_cast<int16>(Scaled * 32767.0f);
        }

        USoundWaveProcedural* Wave = NewObject<USoundWaveProcedural>(GetTransientPackage());
        if (!Wave)
        {
            return nullptr;
        }

        Wave->NumChannels = 1;
        Wave->SampleByteSize = sizeof(int16);
        Wave->SetSampleRate(RIFallbackSampleRate, false);
        Wave->SetNumFrames(SampleCount);
        Wave->Duration = Duration;
        Wave->QueueAudio(
            reinterpret_cast<const uint8*>(PCM.GetData()),
            PCM.Num() * static_cast<int32>(sizeof(int16)));

        ActiveFallbackSounds.Emplace(Wave, FPlatformTime::Seconds() + static_cast<double>(Duration) + 1.0);
        return Wave;
    }
}

namespace RIAudioEvents
{
    void Play(UObject* WorldContext, const FName EventName, const FVector& WorldLocation, const float Volume, const float Pitch)
    {
        if (!WorldContext || EventName.IsNone()) return;

        PruneFallbackSounds();

        USoundBase* Sound = nullptr;
        bool bUsingFallback = false;
        if (!MissingAudioEvents.Contains(EventName))
        {
            const FString AssetName = FString::Printf(TEXT("SFX_%s"), *EventName.ToString());
            const FString AssetPath = FString::Printf(TEXT("/Game/Audio/SFX/%s.%s"), *AssetName, *AssetName);
            Sound = LoadObject<USoundBase>(nullptr, *AssetPath);
            if (!Sound)
            {
                MissingAudioEvents.Add(EventName);
            }
        }

        if (!Sound)
        {
            Sound = BuildFallbackWave(EventName);
            bUsingFallback = Sound != nullptr;
        }

        if (!Sound) return;

        float EffectiveVolume = FMath::Clamp(Volume, 0.0f, 2.5f);
        if (bUsingFallback)
        {
            if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(WorldContext, 0))
            {
                const float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), WorldLocation);
                const float DistanceAlpha = FMath::Clamp(Distance / 6500.0f, 0.0f, 1.0f);
                EffectiveVolume *= FMath::Lerp(1.0f, 0.12f, DistanceAlpha);
            }
        }

        UGameplayStatics::PlaySoundAtLocation(
            WorldContext,
            Sound,
            WorldLocation,
            EffectiveVolume,
            FMath::Clamp(Pitch, 0.55f, 1.75f));
    }
}
