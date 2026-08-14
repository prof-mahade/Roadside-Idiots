#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RIBananaPeelHazard.generated.h"

class ARIBikePawn;
class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class ROADSIDEIDIOTS_API ARIBananaPeelHazard : public AActor
{
    GENERATED_BODY()

public:
    ARIBananaPeelHazard();
    virtual void BeginPlay() override;

    // Called before FinishSpawning. The dropper is immune only briefly so the
    // freshly dropped peel cannot hit them instantly, but they can still circle
    // back later and suffer the deserved self-own.
    void ConfigureSource(ARIBikePawn* InSourceBike);

private:
    UFUNCTION()
    void HandleHazardOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    // Small physical body: gravity makes the peel visibly fall to the road,
    // while the larger child trigger remains responsible for rider detection.
    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<USphereComponent> PhysicsBody;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<USphereComponent> Trigger;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UStaticMeshComponent> Visual;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UPointLightComponent> Glow;

    TWeakObjectPtr<ARIBikePawn> SourceBike;
    bool bTriggered = false;
    double SourceImmunityEndsAt = 0.0;

    UPROPERTY(EditDefaultsOnly, Category="Banana")
    float SourceImmunitySeconds = 1.0f;
};
