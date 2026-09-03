#include "Presentation/RIPresentationWorldSubsystem.h"

#include "Audio/RIAudioEvents.h"
#include "Core/RIParticipantComponent.h"
#include "Race/RIRaceManager.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Vehicle/RIBikePawn.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"

namespace
{
    constexpr uint32 RIEngineSampleRate = 22050;
    constexpr float RIEngineChunkSeconds = 0.12f;
    constexpr float RIEngineBufferedSeconds = 0.30f;

    float RIEngineNoise(uint32& State)
    {
        State = State * 1664525u + 1013904223u;
        const uint32 Bits = (State >> 8u) & 0xFFFFu;
        return static_cast<float>(Bits) / 32767.5f - 1.0f;
    }
}

bool URIPresentationWorldSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIPresentationWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIPresentationWorldSubsystem, STATGROUP_Tickables);
}

void URIPresentationWorldSubsystem::Deinitialize()
{
    StopPersistentEngineChannel();
    Super::Deinitialize();
}

ARIBikePawn* URIPresentationWorldSubsystem::FindHumanBike() const
{
    if (CachedHumanBike.IsValid())
    {
        return CachedHumanBike.Get();
    }

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        ARIBikePawn* Bike = *It;
        const URIParticipantComponent* Participant = Bike ? Bike->GetParticipantComponent() : nullptr;
        if (Bike && Participant && Participant->IsHumanControlled())
        {
            CachedHumanBike = Bike;
            return Bike;
        }
    }

    return nullptr;
}

void URIPresentationWorldSubsystem::UpdateRaceCues()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!CachedRaceManager)
    {
        for (TActorIterator<ARIRaceManager> It(World); It; ++It)
        {
            CachedRaceManager = *It;
            break;
        }
    }

    if (!CachedRaceManager) return;

    ARIBikePawn* HumanBike = FindHumanBike();
    const FVector CueLocation = HumanBike ? HumanBike->GetActorLocation() : FVector::ZeroVector;
    const float SecondsUntilStart = CachedRaceManager->GetSecondsUntilStart();

    if (SecondsUntilStart > 0.0f)
    {
        const int32 Count = FMath::Clamp(FMath::CeilToInt(SecondsUntilStart), 1, 3);
        if (Count != LastCountdownNumber)
        {
            LastCountdownNumber = Count;
            const float Pitch = 0.92f + static_cast<float>(3 - Count) * 0.06f;
            RIAudioEvents::Play(this, TEXT("Countdown"), CueLocation, 0.95f, Pitch);
        }
        bPlayedGoCue = false;
    }
    else if (CachedRaceManager->IsRaceStarted() && !bPlayedGoCue)
    {
        bPlayedGoCue = true;
        LastCountdownNumber = INDEX_NONE;
        RIAudioEvents::Play(this, TEXT("RaceGo"), CueLocation, 1.0f, 1.0f);
    }

    if (!HumanBike || !HumanBike->GetParticipantComponent()) return;

    FRIRaceProgress HumanProgress;
    const FName HumanId = HumanBike->GetParticipantComponent()->GetParticipantId();
    if (!CachedRaceManager->GetProgress(HumanId, HumanProgress)) return;

    bHumanFinished = HumanProgress.bFinished;

    if (!bLapStateInitialized)
    {
        LastHumanCompletedLaps = HumanProgress.CompletedLaps;
        bLapStateInitialized = true;
    }

    if (HumanProgress.bFinished)
    {
        if (!bPlayedFinishCue)
        {
            bPlayedFinishCue = true;
            RIAudioEvents::Play(this, TEXT("Finish"), CueLocation, 1.0f, 1.0f);
        }
    }
    else if (HumanProgress.CompletedLaps > LastHumanCompletedLaps)
    {
        RIAudioEvents::Play(this, TEXT("LapComplete"), CueLocation, 0.95f, 1.0f);
    }

    LastHumanCompletedLaps = FMath::Max(LastHumanCompletedLaps, HumanProgress.CompletedLaps);
}

void URIPresentationWorldSubsystem::UpdateCrashCues()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TSet<TWeakObjectPtr<ARIBikePawn>> SeenBikes;
    ARIBikePawn* HumanBike = FindHumanBike();
    const double Now = World->GetTimeSeconds();

    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        ARIBikePawn* Bike = *It;
        if (!Bike) continue;

        const TWeakObjectPtr<ARIBikePawn> Key(Bike);
        SeenBikes.Add(Key);
        const bool bTipped = Bike->GetActorUpVector().Z < 0.38f;
        const bool bWasTipped = LastTippedState.FindRef(Key);

        if (bTipped && !bWasTipped && Bike->AreRaceControlsEnabled())
        {
            if (Bike == HumanBike)
            {
                RIAudioEvents::Play(
                    this,
                    TEXT("Crash"),
                    Bike->GetActorLocation(),
                    0.95f,
                    FMath::FRandRange(0.92f, 1.05f));
            }
            else if (
                HumanBike &&
                FVector::DistSquared2D(HumanBike->GetActorLocation(), Bike->GetActorLocation()) <= FMath::Square(2200.0f) &&
                Now - LastRivalCrashCueTime >= 0.22)
            {
                // Nearby rival crashes remain audible, but one pack pile-up can no
                // longer spawn a chorus of crash voices in the same audio frame.
                RIAudioEvents::Play(
                    this,
                    TEXT("Crash"),
                    Bike->GetActorLocation(),
                    0.48f,
                    FMath::FRandRange(0.94f, 1.07f));
                LastRivalCrashCueTime = Now;
            }
        }

        LastTippedState.Add(Key, bTipped);
    }

    for (auto It = LastTippedState.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid() || !SeenBikes.Contains(It.Key()))
        {
            It.RemoveCurrent();
        }
    }
}

void URIPresentationWorldSubsystem::QueueEngineAudioIfNeeded()
{
    if (!EngineProceduralWave) return;

    const int32 TargetBufferedBytes = FMath::RoundToInt(
        static_cast<float>(RIEngineSampleRate) *
        RIEngineBufferedSeconds *
        static_cast<float>(sizeof(int16)));

    int32 SafetyChunks = 0;
    while (EngineProceduralWave->GetAvailableAudioByteCount() < TargetBufferedBytes && SafetyChunks < 4)
    {
        const int32 SampleCount = FMath::Max(
            1,
            FMath::RoundToInt(RIEngineChunkSeconds * static_cast<float>(RIEngineSampleRate)));

        TArray<int16> PCM;
        PCM.SetNumUninitialized(SampleCount);

        for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
        {
            constexpr float BaseFrequency = 92.0f;
            EngineWavePhase += 2.0f * PI * BaseFrequency / static_cast<float>(RIEngineSampleRate);
            if (EngineWavePhase > 2.0f * PI)
            {
                EngineWavePhase = FMath::Fmod(EngineWavePhase, 2.0f * PI);
            }

            const float Noise = RIEngineNoise(EngineNoiseState);
            const float Fundamental = FMath::Sin(EngineWavePhase);
            const float Harmonic2 = FMath::Sin(EngineWavePhase * 2.0f);
            const float Harmonic3 = FMath::Sin(EngineWavePhase * 3.0f);
            const float MechanicalPulse = FMath::Sin(EngineWavePhase * 0.50f);
            const float Value =
                0.54f * Fundamental +
                0.23f * Harmonic2 +
                0.10f * Harmonic3 +
                0.08f * MechanicalPulse +
                0.05f * Noise;

            PCM[SampleIndex] = static_cast<int16>(FMath::Clamp(Value * 0.42f, -0.95f, 0.95f) * 32767.0f);
        }

        EngineProceduralWave->QueueAudio(
            reinterpret_cast<const uint8*>(PCM.GetData()),
            PCM.Num() * static_cast<int32>(sizeof(int16)));
        ++SafetyChunks;
    }
}

void URIPresentationWorldSubsystem::EnsurePersistentEngineChannel(ARIBikePawn* HumanBike)
{
    if (!HumanBike || !HumanBike->GetChassis()) return;

    if (EngineAudioComponent && EngineProceduralWave)
    {
        QueueEngineAudioIfNeeded();
        if (!EngineAudioComponent->IsPlaying())
        {
            EngineAudioComponent->Play(0.0f);
        }
        return;
    }

    StopPersistentEngineChannel();

    EngineProceduralWave = NewObject<USoundWaveProcedural>(this, TEXT("RIEngineProceduralWave"));
    if (!EngineProceduralWave) return;

    EngineProceduralWave->NumChannels = 1;
    EngineProceduralWave->SampleByteSize = sizeof(int16);
    EngineProceduralWave->SetSampleRate(RIEngineSampleRate, false);
    // The FIFO is continuously replenished below. A long duration keeps the
    // component alive instead of treating each engine beat as a new sound.
    EngineProceduralWave->Duration = 3600.0f;

    EngineWavePhase = 0.0f;
    EngineNoiseState = 0x51A7C3D9u;
    QueueEngineAudioIfNeeded();

    EngineAudioComponent = UGameplayStatics::SpawnSoundAttached(
        EngineProceduralWave,
        HumanBike->GetChassis(),
        NAME_None,
        FVector::ZeroVector,
        EAttachLocation::KeepRelativeOffset,
        true,
        0.0f,
        EngineCurrentPitch,
        0.0f,
        nullptr,
        nullptr,
        false);

    if (!EngineAudioComponent)
    {
        EngineProceduralWave = nullptr;
        return;
    }

    // Protect the foundational vehicle note from voice stealing by short-lived
    // horns, impacts and item sounds. Those effects layer on top; they do not own
    // or restart this component.
    EngineAudioComponent->bOverridePriority = true;
    EngineAudioComponent->Priority = 4.0f;
    EngineAudioComponent->bShouldRemainActiveIfDropped = true;
    EngineAudioComponent->SetUISound(false);
    EngineAudioComponent->SetVolumeMultiplier(0.0f);
    EngineAudioComponent->SetPitchMultiplier(EngineCurrentPitch);

    if (!bLoggedPersistentEngine)
    {
        bLoggedPersistentEngine = true;
        UE_LOG(
            LogTemp,
            Display,
            TEXT("RI AUDIO ENGINE channel=persistent_procedural priority=4 remain_active_if_dropped=1 transient_owner=RIAudioEvents"));
    }
}

void URIPresentationWorldSubsystem::StopPersistentEngineChannel()
{
    if (EngineAudioComponent)
    {
        EngineAudioComponent->Stop();
        EngineAudioComponent = nullptr;
    }

    if (EngineProceduralWave)
    {
        EngineProceduralWave->ResetAudio();
        EngineProceduralWave = nullptr;
    }

    EngineCurrentVolume = 0.0f;
    EngineCurrentPitch = 0.82f;
}

void URIPresentationWorldSubsystem::UpdateVehicleAudio(const float DeltaTime)
{
    ARIBikePawn* HumanBike = FindHumanBike();
    if (!HumanBike)
    {
        return;
    }

    URIBikeMovementComponent* Movement = HumanBike->GetBikeMovement();
    if (!Movement)
    {
        return;
    }

    EnsurePersistentEngineChannel(HumanBike);
    QueueEngineAudioIfNeeded();

    const float SpeedKph = FMath::Abs(Movement->GetForwardSpeedKph());
    const float SpeedAlpha = FMath::Clamp(SpeedKph / 140.0f, 0.0f, 1.0f);
    const float Throttle = FMath::Abs(Movement->GetThrottleInput());
    const float Steering = FMath::Abs(Movement->GetSteeringInput());
    const float Brake = Movement->GetBrakeInput();

    const bool bRaceRunning = HumanBike->AreRaceControlsEnabled() && !bHumanFinished;
    const bool bCountdownIdle = CachedRaceManager && !bHumanFinished && CachedRaceManager->GetSecondsUntilStart() > 0.0f;

    const float TargetVolume = bRaceRunning
        ? 0.23f + 0.21f * SpeedAlpha + 0.07f * Throttle
        : (bCountdownIdle ? 0.11f : 0.0f);
    const float TargetPitch = bRaceRunning
        ? 0.78f + 0.70f * SpeedAlpha + 0.08f * Throttle
        : 0.78f;

    EngineCurrentVolume = FMath::FInterpTo(EngineCurrentVolume, TargetVolume, DeltaTime, 7.5f);
    EngineCurrentPitch = FMath::FInterpTo(EngineCurrentPitch, TargetPitch, DeltaTime, 8.5f);

    if (EngineAudioComponent)
    {
        EngineAudioComponent->SetVolumeMultiplier(FMath::Clamp(EngineCurrentVolume, 0.0f, 0.62f));
        EngineAudioComponent->SetPitchMultiplier(FMath::Clamp(EngineCurrentPitch, 0.65f, 1.62f));
    }

    SkidCueCooldown = FMath::Max(0.0f, SkidCueCooldown - DeltaTime);
    const bool bHardTurn = bRaceRunning && SpeedKph > 38.0f && Steering > 0.76f;
    const bool bHardBrake = bRaceRunning && SpeedKph > 32.0f && Brake > 0.72f;
    if ((bHardTurn || bHardBrake) && SkidCueCooldown <= 0.0f)
    {
        const float Severity = FMath::Clamp(
            FMath::Max(Steering, Brake) * (0.45f + SpeedAlpha * 0.55f),
            0.0f,
            1.0f);
        RIAudioEvents::Play(
            this,
            TEXT("TireSkid"),
            HumanBike->GetActorLocation(),
            0.10f + Severity * 0.18f,
            FMath::Lerp(0.92f, 1.12f, Severity));
        SkidCueCooldown = 0.19f;
    }
}

void URIPresentationWorldSubsystem::Tick(const float DeltaTime)
{
    UpdateRaceCues();
    UpdateVehicleAudio(DeltaTime);

    CrashScanAccumulator += DeltaTime;
    if (CrashScanAccumulator >= 0.08f)
    {
        CrashScanAccumulator = 0.0f;
        UpdateCrashCues();
    }
}
