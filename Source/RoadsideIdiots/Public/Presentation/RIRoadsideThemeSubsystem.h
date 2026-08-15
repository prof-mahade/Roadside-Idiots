#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIRoadsideThemeSubsystem.generated.h"

class AActor;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;

UCLASS()
class ROADSIDEIDIOTS_API URIRoadsideThemeSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void TryBuild();
    void BuildFields();
    void BuildUtilityLines();
    void BuildRoadsideClusters();

    UInstancedStaticMeshComponent* CreateGroup(UStaticMesh* Mesh, const FLinearColor& Color);
    void AddInstance(
        UInstancedStaticMeshComponent* Group,
        const FVector& Location,
        const FRotator& Rotation,
        const FVector& Scale);

    FVector RoutePoint(float AngleRadians, float Height = 0.0f) const;
    FVector RouteTangent(float AngleRadians) const;

    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY()
    TObjectPtr<UStaticMesh> SphereMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BasicMaterial;

    UPROPERTY()
    TObjectPtr<AActor> ThemeRoot;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> DirtInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> FieldInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> WaterInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> BrickInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> PlasterInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> TinBlueInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> TinRedInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> ShopOrangeInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> ShopGreenInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> DarkInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> ConcreteInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> LeafInstances;

    bool bBuilt = false;
};
