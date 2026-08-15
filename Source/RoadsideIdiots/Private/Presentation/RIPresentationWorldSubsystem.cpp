#include "Presentation/RIPresentationWorldSubsystem.h"

#include "Audio/RIAudioEvents.h"
#include "Core/RIParticipantComponent.h"
#include "Race/RIRaceManager.h"
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
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        ARIBikePawn* Bike = *It;
        const URIParticipantComponent* Participant = Bike ? Bike->GetParticipantComponent() : nullptr;
        if (Bike && Participant && Participant->IsHumanControlled())
        {
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

void URIPresentationWorldSubsystem::Tick(const float DeltaTime)
{
    UpdateRaceCues();

    CrashScanAccumulator += DeltaTime;
    if (CrashScanAccumulator >= 0.08f)
    {
        CrashScanAccumulator = 0.0f;
        UpdateCrashCues();
    }
}
