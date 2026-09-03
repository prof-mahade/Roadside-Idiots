#pragma once

#include "CoreMinimal.h"
#include "Debug/RIDebugHUD.h"
#include "RIRaceSetupHUD.generated.h"

UCLASS()
class ROADSIDEIDIOTS_API ARIRaceSetupHUD : public ARIDebugHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    void DrawRaceSetupMenu();
    void DrawPauseMenu();
    void DrawSettingsMenu();
    void DrawGameplayMenuHints();
};
