#include "Core/RIPlayerController.h"

#include "Core/RIGameMode.h"
#include "Core/RIRaceSettingsSubsystem.h"
#include "Components/InputComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ARIPlayerController::ARIPlayerController()
{
    bShowMouseCursor = false;
}

void ARIPlayerController::BeginPlay()
{
    Super::BeginPlay();

    MenuMode = ERIMenuMode::RaceSetup;
    ReturnMenuAfterSettings = ERIMenuMode::RaceSetup;
    SelectedMenuRow = 0;
    SetInputMode(FInputModeGameOnly());

    if (!GetWorld()) return;

    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    URIRaceSettingsSubsystem* Settings = GameInstance
        ? GameInstance->GetSubsystem<URIRaceSettingsSubsystem>()
        : nullptr;

    // Pause-menu Restart Race reloads the map but carries a one-shot request in
    // the GameInstance subsystem so the exact configured race starts directly.
    if (Settings && Settings->ConsumeAutoStartAfterReload())
    {
        if (ARIGameMode* GameMode = GetWorld()->GetAuthGameMode<ARIGameMode>())
        {
            MenuMode = ERIMenuMode::None;
            GameMode->StartConfiguredRace();
        }
    }
}

void ARIPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputComponent) return;

    auto ConfigureBinding = [](FInputKeyBinding& Binding)
    {
        // Menu keys remain usable while paused. They do not consume gameplay
        // input, so the existing pawn bindings keep working when no menu is open.
        Binding.bConsumeInput = false;
        Binding.bExecuteWhenPaused = true;
    };

    {
        FInputKeyBinding& Binding = InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &ARIPlayerController::MenuPrevious);
        ConfigureBinding(Binding);
    }
    {
        FInputKeyBinding& Binding = InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &ARIPlayerController::MenuNext);
        ConfigureBinding(Binding);
    }
    {
        FInputKeyBinding& Binding = InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &ARIPlayerController::MenuDecrease);
        ConfigureBinding(Binding);
    }
    {
        FInputKeyBinding& Binding = InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &ARIPlayerController::MenuIncrease);
        ConfigureBinding(Binding);
    }
    {
        FInputKeyBinding& Binding = InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ARIPlayerController::MenuConfirm);
        ConfigureBinding(Binding);
    }
    {
        FInputKeyBinding& Binding = InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ARIPlayerController::TogglePauseMenu);
        ConfigureBinding(Binding);
    }
    {
        // PIE often reserves Escape, so P is an editor-friendly equivalent.
        FInputKeyBinding& Binding = InputComponent->BindKey(EKeys::P, IE_Pressed, this, &ARIPlayerController::TogglePauseMenu);
        ConfigureBinding(Binding);
    }
}

int32 ARIPlayerController::GetCurrentMenuRowCount() const
{
    switch (MenuMode)
    {
    case ERIMenuMode::RaceSetup: return 6;
    case ERIMenuMode::Pause: return 5;
    case ERIMenuMode::Settings: return 3;
    default: return 0;
    }
}

void ARIPlayerController::MenuPrevious()
{
    const int32 RowCount = GetCurrentMenuRowCount();
    if (RowCount <= 0) return;
    SelectedMenuRow = (SelectedMenuRow - 1 + RowCount) % RowCount;
}

void ARIPlayerController::MenuNext()
{
    const int32 RowCount = GetCurrentMenuRowCount();
    if (RowCount <= 0) return;
    SelectedMenuRow = (SelectedMenuRow + 1) % RowCount;
}

void ARIPlayerController::MenuDecrease()
{
    AdjustSelectedSetting(-1);
}

void ARIPlayerController::MenuIncrease()
{
    AdjustSelectedSetting(1);
}

void ARIPlayerController::AdjustSelectedSetting(const int32 Delta)
{
    if (Delta == 0) return;

    if (MenuMode == ERIMenuMode::RaceSetup)
    {
        if (SelectedMenuRow >= 3 || !GetWorld()) return;

        UGameInstance* GameInstance = GetWorld()->GetGameInstance();
        URIRaceSettingsSubsystem* Settings = GameInstance
            ? GameInstance->GetSubsystem<URIRaceSettingsSubsystem>()
            : nullptr;
        if (!Settings) return;

        if (SelectedMenuRow == 0)
        {
            Settings->SetOpponentCount(Settings->GetOpponentCount() + Delta);
        }
        else if (SelectedMenuRow == 1)
        {
            Settings->SetLapCount(Settings->GetLapCount() + Delta);
        }
        else if (SelectedMenuRow == 2)
        {
            Settings->SetTrafficCount(Settings->GetTrafficCount() + Delta);
        }
        return;
    }

    if (MenuMode != ERIMenuMode::Settings) return;

    UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
    if (!UserSettings) return;

    if (SelectedMenuRow == 0)
    {
        int32 Quality = UserSettings->GetOverallScalabilityLevel();
        if (Quality < 0 || Quality > 3)
        {
            Quality = 2;
        }

        Quality = FMath::Clamp(Quality + Delta, 0, 3);
        UserSettings->SetOverallScalabilityLevel(Quality);
        UserSettings->ApplySettings(false);
    }
    else if (SelectedMenuRow == 1)
    {
        UserSettings->SetVSyncEnabled(!UserSettings->IsVSyncEnabled());
        UserSettings->ApplySettings(false);
    }
}

void ARIPlayerController::MenuConfirm()
{
    if (!GetWorld()) return;

    if (MenuMode == ERIMenuMode::RaceSetup)
    {
        if (SelectedMenuRow == 3)
        {
            ARIGameMode* GameMode = GetWorld()->GetAuthGameMode<ARIGameMode>();
            if (!GameMode) return;

            MenuMode = ERIMenuMode::None;
            SelectedMenuRow = 0;
            GameMode->StartConfiguredRace();
            SetInputMode(FInputModeGameOnly());
        }
        else if (SelectedMenuRow == 4)
        {
            OpenSettingsMenu();
        }
        else if (SelectedMenuRow == 5)
        {
            QuitToDesktop();
        }
        return;
    }

    if (MenuMode == ERIMenuMode::Pause)
    {
        switch (SelectedMenuRow)
        {
        case 0: ResumeGame(); break;
        case 1: RestartConfiguredRace(); break;
        case 2: ReturnToRaceSetup(); break;
        case 3: OpenSettingsMenu(); break;
        case 4: QuitToDesktop(); break;
        default: break;
        }
        return;
    }

    if (MenuMode == ERIMenuMode::Settings && SelectedMenuRow == 2)
    {
        ReturnFromSettings();
    }
}

void ARIPlayerController::TogglePauseMenu()
{
    if (MenuMode == ERIMenuMode::RaceSetup)
    {
        // Do not accidentally leave the title/setup screen via a pause key.
        return;
    }

    if (MenuMode == ERIMenuMode::Settings)
    {
        ReturnFromSettings();
        return;
    }

    if (MenuMode == ERIMenuMode::Pause)
    {
        ResumeGame();
        return;
    }

    if (MenuMode == ERIMenuMode::None && GetPawn())
    {
        if (UGameplayStatics::SetGamePaused(this, true))
        {
            MenuMode = ERIMenuMode::Pause;
            SelectedMenuRow = 0;
            SetInputMode(FInputModeGameOnly());
        }
    }
}

void ARIPlayerController::OpenSettingsMenu()
{
    if (MenuMode != ERIMenuMode::RaceSetup && MenuMode != ERIMenuMode::Pause) return;

    ReturnMenuAfterSettings = MenuMode;
    MenuMode = ERIMenuMode::Settings;
    SelectedMenuRow = 0;
}

void ARIPlayerController::ReturnFromSettings()
{
    MenuMode = ReturnMenuAfterSettings;
    SelectedMenuRow = 0;
}

void ARIPlayerController::ResumeGame()
{
    UGameplayStatics::SetGamePaused(this, false);
    MenuMode = ERIMenuMode::None;
    SelectedMenuRow = 0;
    SetInputMode(FInputModeGameOnly());
}

void ARIPlayerController::ReloadCurrentLevel(const bool bAutoStartConfiguredRace)
{
    if (!GetWorld()) return;

    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    URIRaceSettingsSubsystem* Settings = GameInstance
        ? GameInstance->GetSubsystem<URIRaceSettingsSubsystem>()
        : nullptr;

    if (bAutoStartConfiguredRace && Settings)
    {
        Settings->RequestAutoStartAfterReload();
    }

    UGameplayStatics::SetGamePaused(this, false);

    const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    if (!LevelName.IsEmpty())
    {
        UGameplayStatics::OpenLevel(this, FName(*LevelName), false);
    }
}

void ARIPlayerController::RestartConfiguredRace()
{
    ReloadCurrentLevel(true);
}

void ARIPlayerController::ReturnToRaceSetup()
{
    ReloadCurrentLevel(false);
}

void ARIPlayerController::QuitToDesktop()
{
    UGameplayStatics::SetGamePaused(this, false);
    UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
