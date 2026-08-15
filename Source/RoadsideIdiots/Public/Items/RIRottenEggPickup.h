#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RIRottenEggPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class ROADSIDEIDIOTS_API ARIRottenEggPickup : public AActor
{
    GENERATED_BODY()

public:
    ARIRottenEggPickup();

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void HandleOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USphereComponent> Trigger;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Visual;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UPointLightComponent> Glow;
};
