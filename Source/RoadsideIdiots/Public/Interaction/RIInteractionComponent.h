#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RIInteractionComponent.generated.h"

UCLASS(ClassGroup=(RoadsideIdiots), meta=(BlueprintSpawnableComponent))
class ROADSIDEIDIOTS_API URIInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URIInteractionComponent();

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Interaction")
    bool TrySideInteraction(float Side);

private:
    UPROPERTY(EditAnywhere, Category="Tuning")
    float Reach = 215.0f;

    UPROPERTY(EditAnywhere, Category="Tuning")
    float Radius = 65.0f;

    UPROPERTY(EditAnywhere, Category="Tuning")
    float SideVelocityChange = 270.0f;

    UPROPERTY(EditAnywhere, Category="Tuning")
    float ImpactCost = 7.0f;

    UPROPERTY(EditAnywhere, Category="Tuning")
    float CooldownSeconds = 0.55f;

    double LastUseTime = -100.0;
};
