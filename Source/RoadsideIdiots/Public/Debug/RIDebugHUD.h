#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RIDebugHUD.generated.h"

class ARIRaceManager;

UCLASS()
class ROADSIDEIDIOTS_API ARIDebugHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
private:
    UPROPERTY() TObjectPtr<ARIRaceManager> CachedRaceManager;
};
