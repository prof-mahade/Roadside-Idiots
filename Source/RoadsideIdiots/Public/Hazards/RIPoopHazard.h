#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RIPoopHazard.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class ARIBikePawn;

UENUM()
enum class ERIPoopHazardType : uint8
{
    Dog,
    Cow
};

UCLASS()
class ROADSIDEIDIOTS_API ARIPoopHazard : public AActor
{
    GENERATED_BODY()

public:
    ARIPoopHazard();

    virtual void BeginPlay() override;

    void Configure(ERIPoopHazardType InType);
    ERIPoopHazardType GetHazardType() const { return HazardType; }

private:
    UFUNCTION()
    void HandleOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    void ApplyPresentation();
    void ApplyDogPoop(ARIBikePawn* Bike);
    void ApplyCowPoop(ARIBikePawn* Bike);
    void SpawnMessEffect(ARIBikePawn* Bike, bool bCowMess, float LifetimeSeconds);

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBoxComponent> TriggerVolume;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> BlobA;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> BlobB;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> BlobC;

    // Subtle world-space readability cue. It is intentionally much weaker than
    // pickup glows: hazards should be noticed at speed, not look like collectibles.
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UPointLightComponent> StinkGlow;

    ERIPoopHazardType HazardType = ERIPoopHazardType::Dog;
    TMap<TWeakObjectPtr<ARIBikePawn>, double> LastTriggerTimes;
};