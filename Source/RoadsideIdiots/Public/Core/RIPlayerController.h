#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RIPlayerController.generated.h"

enum class ERIMenuMode : uint8
{
    None,
    RaceSetup,
    Pause,
    Settings
};

UCLASS()
class ROADSIDEIDIOTS_API ARIPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ARIPlayerController();

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    bool IsRaceSetupMenuOpen() const { return MenuMode == ERIMenuMode::RaceSetup; }
    bool IsPauseMenuOpen() const { return MenuMode == ERIMenuMode::Pause; }
    bool IsSettingsMenuOpen() const { return MenuMode == ERIMenuMode::Settings; }
    bool IsAnyMenuOpen() const { return MenuMode != ERIMenuMode::None; }
    int32 GetSelectedMenuRow() const { return SelectedMenuRow; }

private:
    void MenuPrevious();
    void MenuNext();
    void MenuDecrease();
    void MenuIncrease();
    void MenuConfirm();
    void TogglePauseMenu();

    void AdjustSelectedSetting(int32 Delta);
    int32 GetCurrentMenuRowCount() const;
    void OpenSettingsMenu();
    void ReturnFromSettings();
    void ResumeGame();
    void RestartConfiguredRace();
    void ReturnToRaceSetup();
    void QuitToDesktop();
    void ReloadCurrentLevel(bool bAutoStartConfiguredRace);

    ERIMenuMode MenuMode = ERIMenuMode::RaceSetup;
    ERIMenuMode ReturnMenuAfterSettings = ERIMenuMode::RaceSetup;
    int32 SelectedMenuRow = 0;
};
