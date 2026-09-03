#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIWorldSignageSubsystem.generated.h"

/**
 * Presentation-only text landmarks built from Unreal's built-in TextRender.
 * No external font/assets, collision, navigation or gameplay ownership.
 */
UCLASS()
class ROADSIDEIDIOTS_API URIWorldSignageSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void BuildSigns();

    bool bBuilt = false;
};
