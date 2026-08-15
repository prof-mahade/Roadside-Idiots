#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RIPoopMessEffect.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class ARIBikePawn;

UCLASS()
class ROADSIDEIDIOTS_API ARIPoopMessEffect : public AActor
{
    GENERATED_BODY()

public:
    ARIPoopMessEffect();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    void Configure(ARIBikePawn* InBike, bool bInCowMess, float InLifetimeSeconds);
    ARIBikePawn* GetAffectedBike() const { return AffectedBike.Get(); }
    bool IsCowMess() const { return bCowMess; }

private:
    void ApplyPresentation();
    void UpdateFumes(float AgeSeconds);

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> SplatA;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> SplatB;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> SplatC;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> FumeA;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> FumeB;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> FumeC;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> FumeD;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UPointLightComponent> StinkGlow;

    TWeakObjectPtr<ARIBikePawn> AffectedBike;
    bool bCowMess = false;
    float LifetimeSeconds = 4.0f;
    double SpawnedAt = 0.0;
    double ExpiresAt = 0.0;
};
