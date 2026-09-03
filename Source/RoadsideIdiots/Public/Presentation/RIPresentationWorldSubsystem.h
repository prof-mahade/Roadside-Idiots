#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIPresentationWorldSubsystem.generated.h"

class ARIBikePawn;
class ARIRaceManager;
class UAudioComponent;
class USoundWaveProcedural;

UCLASS()
class ROADSIDEIDIOTS_API URIPresentationWorldSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;
    virtual void Deinitialize() override;

private:
    void UpdateRaceCues();
    void UpdateCrashCues();
    void UpdateVehicleAudio(float DeltaTime);
    void EnsurePersistentEngineChannel(ARIBikePawn* HumanBike);
    void QueueEngineAudioIfNeeded();
    void StopPersistentEngineChannel();
    ARIBikePawn* FindHumanBike() const;

    UPROPERTY()
    TObjectPtr<ARIRaceManager> CachedRaceManager;

    // Avoid a full actor scan every presentation tick once the player bike is found.
    mutable TWeakObjectPtr<ARIBikePawn> CachedHumanBike;

    // The engine is a persistent procedural audio channel. Transient race/item/
    // impact sounds continue to use RIAudioEvents::Play and layer over this voice.
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> EngineAudioComponent;

    UPROPERTY(Transient)
    TObjectPtr<USoundWaveProcedural> EngineProceduralWave;

    TMap<TWeakObjectPtr<ARIBikePawn>, bool> LastTippedState;
    float CrashScanAccumulator = 0.0f;
    float SkidCueCooldown = 0.0f;
    float EngineWavePhase = 0.0f;
    float EngineCurrentVolume = 0.0f;
    float EngineCurrentPitch = 0.82f;
    uint32 EngineNoiseState = 0x51A7C3D9u;
    double LastRivalCrashCueTime = -100.0;
    int32 LastCountdownNumber = INDEX_NONE;
    int32 LastHumanCompletedLaps = 0;
    bool bLapStateInitialized = false;
    bool bPlayedGoCue = false;
    bool bPlayedFinishCue = false;
    bool bHumanFinished = false;
    bool bLoggedPersistentEngine = false;
};
