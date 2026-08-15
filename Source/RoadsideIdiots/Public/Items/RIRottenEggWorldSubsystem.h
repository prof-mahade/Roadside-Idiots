#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIRottenEggWorldSubsystem.generated.h"

class ARIBikePawn;

UCLASS()
class ROADSIDEIDIOTS_API URIRottenEggWorldSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

    void AddEgg(int32 Amount = 1);
    int32 GetEggCount() const { return EggCount; }
    int32 GetMaxEggCount() const { return MaxEggs; }

private:
    void TrySpawnPickups();
    void TryThrowEgg();
    ARIBikePawn* FindHumanBike() const;

    int32 EggCount = 0;
    int32 MaxEggs = 2;
    bool bSpawnedPickups = false;
    double LastThrowTime = -100.0;
};
