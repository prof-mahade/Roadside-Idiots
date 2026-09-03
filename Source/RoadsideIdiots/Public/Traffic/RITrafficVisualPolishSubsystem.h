#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RITrafficVisualPolishSubsystem.generated.h"

class ARITrafficVehicle;
class UMaterialInterface;
class UStaticMesh;

/**
 * Presentation-only detail layer for civilian traffic.
 *
 * The authoritative traffic actor keeps its existing route motion and impact
 * volume. This subsystem only attaches non-colliding detail meshes so taxis,
 * CNGs, vans and microbuses read as vehicles rather than colored boxes.
 */
UCLASS()
class ROADSIDEIDIOTS_API URITrafficVisualPolishSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void EnsurePolish(ARITrafficVehicle* Traffic);

    float ScanAccumulator = 0.0f;
    TSet<TWeakObjectPtr<ARITrafficVehicle>> PolishedTraffic;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> SphereMesh;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> BaseMaterial;
};
