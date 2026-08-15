#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RITrafficStyleSubsystem.generated.h"

class ARITrafficVehicle;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

UCLASS()
class ROADSIDEIDIOTS_API URITrafficStyleSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void TryStyleTraffic();
    void StyleVehicle(ARITrafficVehicle* Vehicle, int32 StyleIndex);
    UStaticMeshComponent* FindMeshComponent(ARITrafficVehicle* Vehicle, FName ComponentName) const;
    UStaticMeshComponent* CreateDetail(
        ARITrafficVehicle* Vehicle,
        FName ComponentName,
        const FVector& RelativeLocation,
        const FRotator& RelativeRotation,
        const FVector& RelativeScale,
        const FLinearColor& Color);
    void SetWheelLayout(
        ARITrafficVehicle* Vehicle,
        float FrontX,
        float RearX,
        float HalfWidth,
        float Height,
        const FVector& WheelScale);

    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BasicMaterial;

    bool bStyled = false;
};
