#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RIGameMode.generated.h"

class ARIRaceManager;
class ARIDemoWorldBuilder;

UCLASS()
class ROADSIDEIDIOTS_API ARIGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ARIGameMode();
protected:
    virtual void BeginPlay() override;
private:
    UPROPERTY() TObjectPtr<ARIRaceManager> RaceManager;
    UPROPERTY() TObjectPtr<ARIDemoWorldBuilder> DemoWorldBuilder;
};
