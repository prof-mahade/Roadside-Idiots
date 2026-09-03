#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIRoadSurfaceDetailSubsystem.generated.h"

/**
 * Presentation-only asphalt detail layer.
 * Adds sparse repair patches and skid streaks a few centimeters above the visual
 * road. Every instance is NoCollision and never becomes an authoritative surface.
 */
UCLASS()
class ROADSIDEIDIOTS_API URIRoadSurfaceDetailSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void BuildSurfaceDetails();

    bool bBuilt = false;
};