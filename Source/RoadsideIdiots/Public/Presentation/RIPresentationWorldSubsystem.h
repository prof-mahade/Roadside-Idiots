#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIPresentationWorldSubsystem.generated.h"

class ARIBikePawn;
class ARIRaceManager;

UCLASS()
class ROADSIDEIDIOTS_API URIPresentationWorldSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void UpdateRaceCues();
    void UpdateCrashCues();
    ARIBikePawn* FindHumanBike() const;

    UPROPERTY()
    TObjectPtr<ARIRaceManager> CachedRaceManager;

    TMap<TWeakObjectPtr<ARIBikePawn>, bool> LastTippedState;
    float CrashScanAccumulator = 0.0f;
    int32 LastCountdownNumber = INDEX_NONE;
    int32 LastHumanCompletedLaps = 0;
    bool bLapStateInitialized = false;
    bool bPlayedGoCue = false;
    bool bPlayedFinishCue = false;
};
