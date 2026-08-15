#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RITrafficVehicle.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class ROADSIDEIDIOTS_API ARITrafficVehicle : public AActor
{
    GENERATED_BODY()

public:
    ARITrafficVehicle();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    void Configure(
        float InStartAngleRadians,
        float InSpeedKph,
        float InLaneOffset,
        const FLinearColor& InBodyColor,
        bool bInWanders,
        float InWanderPhase,
        const FString& InTrafficLabel);

    static FTransform MakeRouteTransform(float AngleRadians, float LaneOffset);

    // Read-only approximation for rival prediction. Traffic movement remains
    // authoritative in this actor; AI only consumes this as perception data.
    FVector GetTrafficVelocityEstimate() const
    {
        return GetActorForwardVector().GetSafeNormal2D() * SpeedCms;
    }

private:
    UFUNCTION()
    void HandleImpactOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    void ApplyVisualMaterials();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBoxComponent> ImpactVolume;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> BodyVisual;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> CabinVisual;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> FrontMarkerLeft;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> FrontMarkerRight;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> RearMarkerLeft;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> RearMarkerRight;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> FrontWheelLeft;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> FrontWheelRight;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> RearWheelLeft;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> RearWheelRight;

    float RouteAngleRadians = 0.0f;
    float SpeedCms = 1200.0f;
    float BaseLaneOffset = 0.0f;
    float WanderPhase = 0.0f;
    bool bWanders = false;
    FLinearColor BodyColor = FLinearColor(0.25f, 0.45f, 0.85f, 1.0f);
    FString TrafficLabel = TEXT("TRAFFIC");

    TMap<TWeakObjectPtr<class ARIBikePawn>, double> LastImpactTimes;
};
