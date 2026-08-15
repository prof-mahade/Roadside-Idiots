#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RITrackPolishSubsystem.generated.h"

class AActor;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;

UCLASS()
class ROADSIDEIDIOTS_API URITrackPolishSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void TryBuild();
    UInstancedStaticMeshComponent* CreateGroup(UStaticMesh* Mesh, const FLinearColor& Color);
    void AddInstance(UInstancedStaticMeshComponent* Group, const FVector& Location, const FRotator& Rotation, const FVector& Scale);
    FVector RoutePoint(float AngleRadians, float Height = 0.0f) const;
    FVector RouteTangent(float AngleRadians) const;

    UPROPERTY()
    TObjectPtr<AActor> PolishRoot;

    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BasicMaterial;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> AsphaltJoinInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> BarrierInsetInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> ChevronYellowInstances;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> ChevronDarkInstances;

    bool bBuilt = false;
};
