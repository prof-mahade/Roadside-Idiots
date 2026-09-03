#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RIRaceManager.generated.h"

USTRUCT(BlueprintType)
struct FRIRaceProgress
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 NextCheckpoint = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 CompletedLaps = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bFinished = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float FinishTime = 0.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float LastCheckpointTime = 0.0f;
};

UCLASS()
class ROADSIDEIDIOTS_API ARIRaceManager : public AActor
{
    GENERATED_BODY()
public:
    ARIRaceManager();
    virtual void BeginPlay() override;

    void ConfigureRace(int32 InCheckpointCount, int32 InTotalLaps = 3);
    void ConfigureCheckpoints(int32 InCheckpointCount) { ConfigureRace(InCheckpointCount, TotalLaps); }
    void RegisterParticipant(FName ParticipantId);
    bool ReportCheckpoint(FName ParticipantId, int32 CheckpointIndex);
    bool GetProgress(FName ParticipantId, FRIRaceProgress& OutProgress) const;
    int32 GetPlace(FName ParticipantId) const;

    int32 GetCheckpointCount() const { return CheckpointCount; }
    int32 GetTotalLaps() const { return TotalLaps; }
    int32 GetParticipantCount() const { return ProgressByParticipant.Num(); }

    bool IsRaceStarted() const;
    float GetSecondsUntilStart() const;
    float GetRaceElapsedTime() const;

private:
    int32 CheckpointCount = 0;
    int32 TotalLaps = 3;
    float CountdownSeconds = 3.0f;
    double RaceStartWorldTime = 0.0;
    TMap<FName, FRIRaceProgress> ProgressByParticipant;
};
