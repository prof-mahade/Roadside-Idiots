#include "Race/RIRaceManager.h"
#include "Engine/World.h"

ARIRaceManager::ARIRaceManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void ARIRaceManager::ConfigureCheckpoints(int32 InCheckpointCount)
{
    CheckpointCount = FMath::Max(0, InCheckpointCount);
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
    if (!Progress || Progress->bFinished || CheckpointCount <= 0 || CheckpointIndex != Progress->NextCheckpoint)
    {
        return false;
    }
    ++Progress->NextCheckpoint;
    if (Progress->NextCheckpoint >= CheckpointCount)
    {
        Progress->bFinished = true;
        Progress->FinishTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
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
        if (Other.bFinished != Target->bFinished) bAhead = Other.bFinished;
        else if (Other.bFinished) bAhead = Other.FinishTime < Target->FinishTime;
        else bAhead = Other.NextCheckpoint > Target->NextCheckpoint;
        if (bAhead) ++Place;
    }
    return Place;
}
