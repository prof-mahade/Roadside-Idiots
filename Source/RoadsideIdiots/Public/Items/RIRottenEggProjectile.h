#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RIRottenEggProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class ARIBikePawn;

UCLASS()
class ROADSIDEIDIOTS_API ARIRottenEggProjectile : public AActor
{
    GENERATED_BODY()

public:
    ARIRottenEggProjectile();
    void ConfigureSource(ARIBikePawn* InSourceBike);

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

    UFUNCTION()
    void HandleWorldHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit);

    void SplatterBike(ARIBikePawn* Victim);

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USphereComponent> Collision;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Visual;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProjectileMovementComponent> Movement;

    TWeakObjectPtr<ARIBikePawn> SourceBike;
    double SourceImmunityEndsAt = 0.0;
    bool bResolved = false;
};
