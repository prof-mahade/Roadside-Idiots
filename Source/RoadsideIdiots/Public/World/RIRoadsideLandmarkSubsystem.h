#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIRoadsideLandmarkSubsystem.generated.h"

class AStaticMeshActor;
class UMaterialInterface;
class UStaticMesh;

/**
 * Presentation-only landmark layer for the Demo 1 oval.
 *
 * The accepted road collision, bike physics and AI route stay authoritative.
 * Everything spawned here has collision disabled and exists only to give the
 * lap stronger place identity, speed reference and start/finish readability.
 */
UCLASS()
class ROADSIDEIDIOTS_API URIRoadsideLandmarkSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void BuildLandmarks();
    void BuildStartFinish();
    void BuildQuarterMarkers();
    void BuildMarketCluster();
    void BuildBusStopCluster();
    void BuildPondAndFields();

    AStaticMeshActor* SpawnDecoration(
        UStaticMesh* Mesh,
        const FVector& Location,
        const FRotator& Rotation,
        const FVector& Scale,
        const FLinearColor& Color);

    bool bBuilt = false;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> SphereMesh;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> CylinderMesh;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> BaseMaterial;
};
