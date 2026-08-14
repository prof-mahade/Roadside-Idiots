#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RIBananaPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class ROADSIDEIDIOTS_API ARIBananaPickup : public AActor
{
    GENERATED_BODY()

public:
    ARIBananaPickup();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

private:
    UFUNCTION()
    void HandlePickupOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<USphereComponent> PickupTrigger;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UStaticMeshComponent> Visual;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UPointLightComponent> Glow;

    bool bConsumed = false;

    UPROPERTY(EditDefaultsOnly, Category="Banana")
    float HealAmount = 12.0f;
};
