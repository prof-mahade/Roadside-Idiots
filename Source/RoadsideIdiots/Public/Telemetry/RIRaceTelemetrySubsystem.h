#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIRaceTelemetrySubsystem.generated.h"

class ARIBikePawn;
class ARIRaceManager;

/**
 * Passive Demo 1 playtest instrumentation.
 *
 * This subsystem only observes already-public race/player state. It never sends
 * control input, changes physics or alters race rules. The summary goes through
 * UE_LOG so normal editor/packaged logs become the evidence file automatically.
 */
UCLASS()
class ROADSIDEIDIOTS_API URIRaceTelemetrySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    void BeginObservation(ARIBikePawn* PlayerBike, ARIRaceManager* RaceManager);
    void SampleRace(ARIBikePawn* PlayerBike, ARIRaceManager* RaceManager);
    void WriteSummary(const TCHAR* Reason);

    TWeakObjectPtr<ARIRaceManager> CachedRaceManager;

    float SampleAccumulator = 0.0f;
    float ObservationStartedAt = 0.0f;
    float SpeedSampleSum = 0.0f;
    float MaxSpeedKph = 0.0f;
    float TotalConditionLost = 0.0f;

    int32 SpeedSampleCount = 0;
    int32 DamageEvents = 0;
    int32 Overtakes = 0;
    int32 PositionsLost = 0;
    int32 BananaPickups = 0;
    int32 EggPickups = 0;
    int32 PeelUses = 0;
    int32 EggUses = 0;

    int32 LastPlace = 0;
    int32 LastBananaCount = 0;
    int32 LastEggCount = 0;
    float LastCondition = 0.0f;

    bool bRaceObserved = false;
    bool bSummaryWritten = false;
};
