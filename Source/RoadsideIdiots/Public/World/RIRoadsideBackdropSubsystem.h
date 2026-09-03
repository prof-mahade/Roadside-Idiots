#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIRoadsideBackdropSubsystem.generated.h"

/**
 * Builds a low-cost distant skyline/tree belt after the configured race world
 * exists. Presentation only: all instances are non-colliding, non-navigable and
 * live well outside the authoritative race corridor.
 */
UCLASS()
class ROADSIDEIDIOTS_API URIRoadsideBackdropSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void BuildBackdrop();

    bool bBuilt = false;
};