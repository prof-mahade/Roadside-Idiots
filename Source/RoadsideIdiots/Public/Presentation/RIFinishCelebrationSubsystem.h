#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIFinishCelebrationSubsystem.generated.h"

class ARIBikePawn;
class ARIRaceManager;
class AStaticMeshActor;
class UMaterialInterface;
class UStaticMesh;

struct FRIConfettiPiece
{
    TWeakObjectPtr<AStaticMeshActor> Actor;
    FVector Velocity = FVector::ZeroVector;
    FRotator AngularVelocityDegrees = FRotator::ZeroRotator;
    float AgeSeconds = 0.0f;
    float LifetimeSeconds = 3.5f;
    float FlutterPhase = 0.0f;
};

/**
 * Lightweight, asset-free finish celebration.
 *
 * It observes the human finish state and animates short-lived, non-colliding
 * confetti pieces kinematically. It never enables body physics, applies force
 * to bikes, changes race state or owns finish audio (that remains in
 * RIPresentationWorldSubsystem).
 */
UCLASS()
class ROADSIDEIDIOTS_API URIFinishCelebrationSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    ARIBikePawn* FindHumanBike() const;
    ARIRaceManager* FindRaceManager() const;
    void SpawnCelebration(const FVector& Origin, const FVector& Forward);
    void UpdateConfetti(float DeltaTime);

    bool bCelebrated = false;
    TArray<FRIConfettiPiece> ConfettiPieces;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> ConfettiMesh;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> BaseMaterial;
};
