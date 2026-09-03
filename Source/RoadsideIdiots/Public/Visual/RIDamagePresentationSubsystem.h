#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIDamagePresentationSubsystem.generated.h"

class ARIBikePawn;
class UStaticMeshComponent;

UCLASS()
class ROADSIDEIDIOTS_API URIDamagePresentationSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void UpdateBike(ARIBikePawn* Bike);
    UStaticMeshComponent* FindStaticComponent(ARIBikePawn* Bike, const FName ComponentName) const;
    UStaticMeshComponent* EnsureAccent(ARIBikePawn* Bike, UStaticMeshComponent* ParentBandage, const FName AccentName, const FVector RelativeScale);
};
