#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RITrackPresentationSubsystem.generated.h"

class ARIBikePawn;
class AStaticMeshActor;
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
    void BuildTrackSkin();
    void BuildStartFinish();
    void BuildRoadsideScenery();
    void UpdateCameraFeel(float DeltaTime);

    AStaticMeshActor* SpawnVisual(
        UStaticMesh* Mesh,
        const FVector& Location,
        const FRotator& Rotation,
        const FVector& Scale,
        const FLinearColor& Color);

    FVector RoutePoint(float AngleRadians, float Height = 0.0f) const;
    FVector RouteTangent(float AngleRadians) const;
    ARIBikePawn* FindHumanBike();

    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY()
    TObjectPtr<UStaticMesh> SphereMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BasicMaterial;

    TWeakObjectPtr<ARIBikePawn> CachedHumanBike;
    bool bBuiltPresentation = false;
};
