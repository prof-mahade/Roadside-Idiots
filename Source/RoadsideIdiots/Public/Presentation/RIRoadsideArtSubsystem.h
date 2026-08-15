#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIRoadsideArtSubsystem.generated.h"

class AActor;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;

UCLASS()
class ROADSIDEIDIOTS_API URIRoadsideArtSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void TryBuild();
    void BuildClusterDetails();
    void BuildFenceDetails();

    UInstancedStaticMeshComponent* CreateColorGroup(UStaticMesh* Mesh, const FLinearColor& Color);
    UInstancedStaticMeshComponent* CreateAssetGroup(UStaticMesh* Mesh);
    void AddInstance(
        UInstancedStaticMeshComponent* Group,
        const FVector& Location,
        const FRotator& Rotation,
        const FVector& Scale);
    void AddAssetFootprintInstance(
        UInstancedStaticMeshComponent* Group,
        const FVector& Location,
        const FRotator& Rotation,
        float DesiredWidthCm,
        float UniformScaleMultiplier = 1.0f);

    FVector RoutePoint(float AngleRadians, float Height = 0.0f) const;
    FVector RouteTangent(float AngleRadians) const;

    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY()
    TObjectPtr<UStaticMesh> CylinderMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BasicMaterial;

    UPROPERTY()
    TObjectPtr<AActor> ArtRoot;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> WoodInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> BambooInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> TrimInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> DoorInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> WindowInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> MetalInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> CrateInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> BarrelInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> GroundBananaAInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> GroundBananaBInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> RottenLeavesInstances;

    bool bBuilt = false;
};
