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
    void UpdateVehicleAudio(float DeltaTime);
    ARIBikePawn* FindHumanBike() const;

    UPROPERTY()
    TObjectPtr<ARIRaceManager> CachedRaceManager;

    // Avoid a full actor scan every presentation tick once the player bike is found.
    mutable TWeakObjectPtr<ARIBikePawn> CachedHumanBike;

    TMap<TWeakObjectPtr<ARIBikePawn>, bool> LastTippedState;
    float CrashScanAccumulator = 0.0f;
    float EnginePulseAccumulator = 0.0f;
    float SkidCueCooldown = 0.0f;
    int32 LastCountdownNumber = INDEX_NONE;
    int32 LastHumanCompletedLaps = 0;
    bool bLapStateInitialized = false;
    bool bPlayedGoCue = false;
    bool bPlayedFinishCue = false;
};
