#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIPoopWorldSubsystem.generated.h"

UCLASS()
class ROADSIDEIDIOTS_API URIPoopWorldSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

    int32 GetSpawnedDogPoopCount() const { return SpawnedDogPoopCount; }
    int32 GetSpawnedCowPoopCount() const { return SpawnedCowPoopCount; }

private:
    void TrySpawnMapHazards();

    bool bSpawned = false;
    int32 SpawnedDogPoopCount = 0;
    int32 SpawnedCowPoopCount = 0;
};
