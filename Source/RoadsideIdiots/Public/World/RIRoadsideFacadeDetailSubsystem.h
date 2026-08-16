#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIRoadsideFacadeDetailSubsystem.generated.h"

/**
 * Small, presentation-only facade details for the near roadside landmarks.
 * Uses Engine basic shapes only and never participates in collision/navigation.
 */
UCLASS()
class ROADSIDEIDIOTS_API URIRoadsideFacadeDetailSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void BuildFacadeDetails();

    bool bBuilt = false;
};