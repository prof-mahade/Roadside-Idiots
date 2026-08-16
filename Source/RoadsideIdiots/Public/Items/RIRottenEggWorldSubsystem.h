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

    int32 GetEggCount() const;
    int32 GetMaxEggCount() const;

private:
    void TrySpawnPickups();
    ARIBikePawn* FindHumanBike() const;

    bool bSpawnedPickups = false;
};
