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
    void RecordComicIncident(const FString& ImpactText);
    void RecordDamageSource(FName SourceTag);
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

    // Player-facing comic incident counts. These include non-damaging events
    // such as poop, so they are intentionally separate from damage-source data.
    int32 CrashIncidents = 0;
    int32 TrafficIncidents = 0;
    int32 SlapIncidents = 0;
    int32 PeelIncidents = 0;
    int32 EggIncidents = 0;
    int32 PoopIncidents = 0;
    int32 OtherIncidents = 0;

    // Accepted health-loss sources. Untagged legacy callers remain visible as
    // UNKNOWN rather than being guessed from presentation text.
    int32 CrashDamageEvents = 0;
    int32 TrafficDamageEvents = 0;
    int32 SlapDamageEvents = 0;
    int32 PeelDamageEvents = 0;
    int32 EggDamageEvents = 0;
    int32 UnknownDamageEvents = 0;

    int32 LastPlace = 0;
    int32 LastBananaCount = 0;
    int32 LastEggCount = 0;
    uint32 LastImpactSerial = 0;

    FString LastImpactText;
    bool bImpactWasActive = false;
    bool bRaceObserved = false;
    bool bSummaryWritten = false;
};
