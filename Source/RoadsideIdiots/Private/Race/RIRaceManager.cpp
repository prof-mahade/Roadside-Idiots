#include "Race/RIRaceManager.h"
#include "Engine/World.h"

ARIRaceManager::ARIRaceManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void ARIRaceManager::BeginPlay()
{
    Super::BeginPlay();
    RaceStartWorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() + CountdownSeconds : CountdownSeconds;
}

void ARIRaceManager::ConfigureRace(int32 InCheckpointCount, int32 InTotalLaps)
{
    CheckpointCount = FMath::Max(0, InCheckpointCount);
    TotalLaps = FMath::Max(1, InTotalLaps);
}

void ARIRaceManager::RegisterParticipant(FName ParticipantId)
{
    if (!ParticipantId.IsNone() && !ProgressByParticipant.Contains(ParticipantId))
    {
        ProgressByParticipant.Add(ParticipantId, FRIRaceProgress{});
    }
}

bool ARIRaceManager::ReportCheckpoint(FName ParticipantId, int32 CheckpointIndex)
{
    FRIRaceProgress* Progress = ProgressByParticipant.Find(ParticipantId);
    if (!Progress || Progress->bFinished || !IsRaceStarted() || CheckpointCount <= 0 || CheckpointIndex != Progress->NextCheckpoint)
    {
        return false;
    }

    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Progress->LastCheckpointTime = Now;
    ++Progress->NextCheckpoint;

    if (Progress->NextCheckpoint >= CheckpointCount)
    {
        ++Progress->CompletedLaps;

        if (Progress->CompletedLaps >= TotalLaps)
        {
            Progress->bFinished = true;
            Progress->FinishTime = Now;
            Progress->NextCheckpoint = CheckpointCount;
        }
        else
        {
            Progress->NextCheckpoint = 0;
        }
    }

    return true;
}

bool ARIRaceManager::GetProgress(FName ParticipantId, FRIRaceProgress& OutProgress) const
{
    if (const FRIRaceProgress* Found = ProgressByParticipant.Find(ParticipantId))
    {
        OutProgress = *Found;
        return true;
    }
    return false;
}

int32 ARIRaceManager::GetPlace(FName ParticipantId) const
{
    const FRIRaceProgress* Target = ProgressByParticipant.Find(ParticipantId);
    if (!Target) return 0;

    int32 Place = 1;
    for (const TPair<FName, FRIRaceProgress>& Pair : ProgressByParticipant)
    {
        if (Pair.Key == ParticipantId) continue;

        const FRIRaceProgress& Other = Pair.Value;
        bool bAhead = false;

        if (Other.bFinished != Target->bFinished)
        {
            bAhead = Other.bFinished;
        }
        else if (Other.bFinished)
        {
            bAhead = Other.FinishTime < Target->FinishTime;
        }
        else if (Other.CompletedLaps != Target->CompletedLaps)
        {
            bAhead = Other.CompletedLaps > Target->CompletedLaps;
        }
        else if (Other.NextCheckpoint != Target->NextCheckpoint)
        {
            bAhead = Other.NextCheckpoint > Target->NextCheckpoint;
        }
        else if (Other.LastCheckpointTime > 0.0f && Target->LastCheckpointTime > 0.0f)
        {
            // When two racers are in the same checkpoint segment, the racer who
            // crossed that checkpoint earlier is probably farther into the segment.
            bAhead = Other.LastCheckpointTime < Target->LastCheckpointTime;
        }

        if (bAhead)
        {
            ++Place;
        }
    }
    return Place;
}

bool ARIRaceManager::IsRaceStarted() const
{
    return GetWorld() && GetWorld()->GetTimeSeconds() >= RaceStartWorldTime;
}

float ARIRaceManager::GetSecondsUntilStart() const
{
    if (!GetWorld()) return CountdownSeconds;
    return FMath::Max(0.0f, static_cast<float>(RaceStartWorldTime - GetWorld()->GetTimeSeconds()));
}

float ARIRaceManager::GetRaceElapsedTime() const
{
    if (!GetWorld()) return 0.0f;
    return FMath::Max(0.0f, static_cast<float>(GetWorld()->GetTimeSeconds() - RaceStartWorldTime));
}
