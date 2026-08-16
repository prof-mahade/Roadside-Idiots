#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIRivalIdentitySubsystem.generated.h"

class ARIBikePawn;
class UMaterialInterface;
class UStaticMesh;

/**
 * Adds compact color-coded tail/fairing accents to AI bikes so personality is
 * readable in motion without relying entirely on HUD text. Presentation only:
 * no collision, physics, control or AI values are changed.
 */
UCLASS()
class ROADSIDEIDIOTS_API URIRivalIdentitySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void EnsureIdentity(ARIBikePawn* Bike, const FString& PersonalityLabel);
    FLinearColor ColorForPersonality(const FString& PersonalityLabel) const;

    float ScanAccumulator = 0.0f;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> BaseMaterial;
};