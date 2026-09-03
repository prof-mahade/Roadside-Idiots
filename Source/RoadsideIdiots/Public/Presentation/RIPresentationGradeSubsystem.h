#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIPresentationGradeSubsystem.generated.h"

/**
 * Mild global presentation grade for the prototype world.
 * Rendering only: no camera transform, gameplay, physics or AI ownership.
 */
UCLASS()
class ROADSIDEIDIOTS_API URIPresentationGradeSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void BuildGrade();

    bool bBuilt = false;
};