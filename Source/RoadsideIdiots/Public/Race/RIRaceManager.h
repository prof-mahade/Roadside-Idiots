#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RIRaceManager.generated.h"

USTRUCT(BlueprintType)
struct FRIRaceProgress
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 NextCheckpoint = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bFinished = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float FinishTime = 0.0f;
};

UCLASS()
class ROADSIDEIDIOTS_API ARIRaceManager : public AActor
{
    GENERATED_BODY()
public:
    ARIRaceManager();
    void ConfigureCheckpoints(int32 InCheckpointCount);
    void RegisterParticipant(FName ParticipantId);
    bool ReportCheckpoint(FName ParticipantId, int32 CheckpointIndex);
    bool GetProgress(FName ParticipantId, FRIRaceProgress& OutProgress) const;
    int32 GetPlace(FName ParticipantId) const;
    int32 GetCheckpointCount() const { return CheckpointCount; }
    int32 GetParticipantCount() const { return ProgressByParticipant.Num(); }
private:
    int32 CheckpointCount = 0;
    TMap<FName, FRIRaceProgress> ProgressByParticipant;
};
