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

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<USphereComponent> Trigger;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UStaticMeshComponent> Visual;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UPointLightComponent> Glow;

    TWeakObjectPtr<ARIBikePawn> SourceBike;
    bool bTriggered = false;
};
