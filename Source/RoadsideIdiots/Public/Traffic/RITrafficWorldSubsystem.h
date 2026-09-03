#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RITrafficWorldSubsystem.generated.h"

UCLASS()
class ROADSIDEIDIOTS_API URITrafficWorldSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void TrySpawnTraffic();

    bool bTrafficSpawned = false;
};
