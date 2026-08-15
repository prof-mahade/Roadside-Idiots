#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RIPlayerController.generated.h"

UCLASS()
class ROADSIDEIDIOTS_API ARIPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ARIPlayerController();

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    bool IsRaceSetupMenuOpen() const { return bRaceSetupMenuOpen; }
    int32 GetSelectedSetupRow() const { return SelectedSetupRow; }

private:
    void SetupPrevious();
    void SetupNext();
    void SetupDecrease();
    void SetupIncrease();
    void SetupConfirm();
    void AdjustSelectedSetting(int32 Delta);

    bool bRaceSetupMenuOpen = true;
    int32 SelectedSetupRow = 0;
};
