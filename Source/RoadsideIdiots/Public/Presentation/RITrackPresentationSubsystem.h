#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RITrackPresentationSubsystem.generated.h"

class AActor;
class ARIBikePawn;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;

UCLASS()
class ROADSIDEIDIOTS_API URITrackPresentationSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void TryBuildPresentation();
    void InitializeInstanceGroups();
    void BuildTrackSkin();
    void BuildStartFinish();
    void BuildRoadsideScenery();
    void UpdateCameraFeel(float DeltaTime);

    UInstancedStaticMeshComponent* CreateInstanceGroup(UStaticMesh* Mesh, const FLinearColor& Color);
    void AddVisualInstance(
        UInstancedStaticMeshComponent* Group,
        const FVector& Location,
        const FRotator& Rotation,
        const FVector& Scale);

    FVector RoutePoint(float AngleRadians, float Height = 0.0f) const;
    FVector RouteTangent(float AngleRadians) const;
    ARIBikePawn* FindHumanBike();

    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY()
    TObjectPtr<UStaticMesh> SphereMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BasicMaterial;

    UPROPERTY()
    TObjectPtr<AActor> PresentationRoot;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> GrassInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> GrassAccentInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> AsphaltInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> ConcreteInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> YellowInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> WhiteInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> DarkInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> TrunkInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> LeafAInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> LeafBInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> RedInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> BlueInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> OrangeInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> PurpleInstances;

    TWeakObjectPtr<ARIBikePawn> CachedHumanBike;
    bool bBuiltPresentation = false;
};
