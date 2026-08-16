#include "Presentation/RIPresentationWorldSubsystem.h"

#include "Audio/RIAudioEvents.h"
#include "Core/RIParticipantComponent.h"
#include "Race/RIRaceManager.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Vehicle/RIBikePawn.h"
#include "EngineUtils.h"

bool URIPresentationWorldSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIPresentationWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIPresentationWorldSubsystem, STATGROUP_Tickables);
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
            RIAudioEvents::Play(this, TEXT("Crash"), Bike->GetActorLocation(), 1.0f, FMath::FRandRange(0.92f, 1.05f));
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

void URIPresentationWorldSubsystem::UpdateVehicleAudio(const float DeltaTime)
{
    ARIBikePawn* HumanBike = FindHumanBike();
    if (!HumanBike || !HumanBike->AreRaceControlsEnabled())
    {
        EnginePulseAccumulator = 0.0f;
        SkidCueCooldown = FMath::Max(0.0f, SkidCueCooldown - DeltaTime);
        return;
    }

    URIBikeMovementComponent* Movement = HumanBike->GetBikeMovement();
    if (!Movement)
    {
        return;
    }

    const float SpeedKph = FMath::Abs(Movement->GetForwardSpeedKph());
    const float SpeedAlpha = FMath::Clamp(SpeedKph / 140.0f, 0.0f, 1.0f);
    const float Throttle = FMath::Abs(Movement->GetThrottleInput());
    const float Steering = FMath::Abs(Movement->GetSteeringInput());
    const float Brake = Movement->GetBrakeInput();

    // Asset-free prototype engine note. A real looping motorcycle asset can later
    // replace SFX_EnginePulse without changing this gameplay/presentation code.
    // Slightly louder than the first single-owner pass, per player feedback,
    // while keeping headroom for impacts, race cues and traffic warnings.
    EnginePulseAccumulator += DeltaTime;
    const float PulseInterval = FMath::Lerp(0.18f, 0.095f, SpeedAlpha);
    if (EnginePulseAccumulator >= PulseInterval)
    {
        EnginePulseAccumulator = FMath::Fmod(EnginePulseAccumulator, PulseInterval);
        const float Volume = 0.19f + 0.23f * SpeedAlpha + 0.06f * Throttle;
        const float Pitch = 0.78f + 0.68f * SpeedAlpha + 0.08f * Throttle;
        RIAudioEvents::Play(this, TEXT("EnginePulse"), HumanBike->GetActorLocation(), Volume, Pitch);
    }

    SkidCueCooldown = FMath::Max(0.0f, SkidCueCooldown - DeltaTime);
    const bool bHardTurn = SpeedKph > 38.0f && Steering > 0.76f;
    const bool bHardBrake = SpeedKph > 32.0f && Brake > 0.72f;
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
