#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RITrafficReadabilitySubsystem.generated.h"

/**
 * Presentation-only traffic warning layer.
 *
 * It observes relative motion and may play a restrained horn when the human
 * rider is rapidly closing on traffic ahead. It never changes traffic motion,
 * bike controls, collision, AI or race state.
 */
UCLASS()
class ROADSIDEIDIOTS_API URITrafficReadabilitySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    float ScanAccumulator = 0.0f;
    double LastWarningTime = -100.0;
};
