#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIRoadMarkingSubsystem.generated.h"

/**
 * Instanced presentation-only road markings for the frozen Demo 1 oval.
 *
 * Markings are rendered slightly above the visual road, have no collision and
 * never participate in navigation, physics, checkpoints or AI route following.
 */
UCLASS()
class ROADSIDEIDIOTS_API URIRoadMarkingSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void BuildMarkings();
    bool bBuilt = false;
};
