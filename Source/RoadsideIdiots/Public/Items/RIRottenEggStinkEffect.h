#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RIRottenEggStinkEffect.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class ROADSIDEIDIOTS_API ARIRottenEggStinkEffect : public AActor
{
    GENERATED_BODY()

public:
    ARIRottenEggStinkEffect();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> PuffA;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> PuffB;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> PuffC;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UPointLightComponent> Glow;
};
