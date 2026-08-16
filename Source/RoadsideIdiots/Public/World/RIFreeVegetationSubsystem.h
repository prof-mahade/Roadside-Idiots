#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIFreeVegetationSubsystem.generated.h"

/**
 * Asset-first roadside vegetation using only the project's approved free
 * PN_tropicalGroundPlants / PN_Banana meshes. Presentation only: every layer
 * disables collision/navigation and sits outside the authoritative race road.
 */
UCLASS()
class ROADSIDEIDIOTS_API URIFreeVegetationSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void BuildVegetation();

    bool bBuilt = false;
};
