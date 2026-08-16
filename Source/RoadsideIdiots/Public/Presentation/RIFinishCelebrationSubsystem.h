#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIFinishCelebrationSubsystem.generated.h"

class ARIBikePawn;
class ARIRaceManager;
class UMaterialInterface;
class UStaticMesh;

/**
 * Lightweight, asset-free finish celebration.
 *
 * It observes the human finish state and spawns short-lived, non-colliding
 * confetti pieces. It never applies force to bikes, changes race state or owns
 * finish audio (that remains in RIPresentationWorldSubsystem).
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

    bool bCelebrated = false;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> ConfettiMesh;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> BaseMaterial;
};
