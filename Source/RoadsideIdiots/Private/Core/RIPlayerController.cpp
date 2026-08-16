#include "Core/RIPlayerController.h"

#include "Core/RIGameMode.h"
#include "Core/RIParticipantComponent.h"
#include "Core/RIRaceSettingsSubsystem.h"
#include "Race/RIRaceManager.h"
#include "Vehicle/RIBikePawn.h"
#include "Components/InputComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
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

    // Pause-menu and post-finish restart reload the map but carry a one-shot
    // request in the GameInstance subsystem so the configured race starts again.
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

    auto ConfigureMenuBinding = [](FInputKeyBinding& Binding)
    {
        // Menu navigation remains available while paused and does not consume
        // gameplay buttons such as A/B when no menu owns the action.
        Binding.bConsumeInput = false;
        Binding.bExecuteWhenPaused = true;
    };

    auto ConfigureExclusiveBinding = [](FInputKeyBinding& Binding)
    {
        // Enter/Y used to leak through to the pawn-level RestartRace action.
        // Consume them here so player-controller restart is the single effective
        // owner and can preserve configured race settings/autostart behavior.
        Binding.bConsumeInput = true;
        Binding.bExecuteWhenPaused = true;
    };

    auto BindMenuKey = [&](const FKey& Key, void (ARIPlayerController::*Function)())
    {
        FInputKeyBinding& Binding = InputComponent->BindKey(Key, IE_Pressed, this, Function);
        ConfigureMenuBinding(Binding);
    };

    auto BindExclusiveKey = [&](const FKey& Key, void (ARIPlayerController::*Function)())
    {
        FInputKeyBinding& Binding = InputComponent->BindKey(Key, IE_Pressed, this, Function);
        ConfigureExclusiveBinding(Binding);
    };

    BindMenuKey(EKeys::Up, &ARIPlayerController::MenuPrevious);
    BindMenuKey(EKeys::Down, &ARIPlayerController::MenuNext);
    BindMenuKey(EKeys::Left, &ARIPlayerController::MenuDecrease);
    BindMenuKey(EKeys::Right, &ARIPlayerController::MenuIncrease);
    BindExclusiveKey(EKeys::Enter, &ARIPlayerController::MenuConfirm);

    // Controller parity: D-pad navigates; A confirms; B backs out/resumes;
    // Start pauses; Y is a post-finish quick-restart shortcut only.
    BindMenuKey(EKeys::Gamepad_DPad_Up, &ARIPlayerController::MenuPrevious);
    BindMenuKey(EKeys::Gamepad_DPad_Down, &ARIPlayerController::MenuNext);
    BindMenuKey(EKeys::Gamepad_DPad_Left, &ARIPlayerController::MenuDecrease);
    BindMenuKey(EKeys::Gamepad_DPad_Right, &ARIPlayerController::MenuIncrease);
    BindMenuKey(EKeys::Gamepad_FaceButton_Bottom, &ARIPlayerController::MenuConfirm);
    BindMenuKey(EKeys::Gamepad_FaceButton_Right, &ARIPlayerController::MenuBack);

    // Support both the keyboard Y key and the physical controller Y/top-face
    // button because the player-facing finish prompt historically says "Y".
    BindExclusiveKey(EKeys::Y, &ARIPlayerController::FinishQuickRestart);
    BindExclusiveKey(EKeys::Gamepad_FaceButton_Top, &ARIPlayerController::FinishQuickRestart);

    BindMenuKey(EKeys::Escape, &ARIPlayerController::TogglePauseMenu);
    // PIE often reserves Escape, so P is an editor-friendly equivalent.
    BindMenuKey(EKeys::P, &ARIPlayerController::TogglePauseMenu);
    BindMenuKey(EKeys::Gamepad_Special_Right, &ARIPlayerController::TogglePauseMenu);

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI INPUT FLOW confirm=ENTER/A finish_restart=ENTER/A/Y back=B pause=START restart_owner=player_controller"));
}

int32 ARIPlayerController::GetCurrentMenuRowCount() const
{
    switch (MenuMode)
    {
    case ERIMenuMode::RaceSetup: return 7;
    case ERIMenuMode::Pause: return 5;
    case ERIMenuMode::Settings: return 4;
    default: return 0;
    }
}

bool ARIPlayerController::IsPlayerRaceFinished() const
{
    UWorld* World = GetWorld();
    ARIBikePawn* Bike = Cast<ARIBikePawn>(GetPawn());
    if (!World || !Bike || !Bike->GetParticipantComponent()) return false;

    const FName PlayerId = Bike->GetParticipantComponent()->GetParticipantId();
    if (PlayerId.IsNone()) return false;

    for (TActorIterator<ARIRaceManager> It(World); It; ++It)
    {
        ARIRaceManager* RaceManager = *It;
        if (!RaceManager) continue;

        FRIRaceProgress Progress;
        if (RaceManager->GetProgress(PlayerId, Progress))
        {
            return Progress.bFinished;
        }
    }

    return false;
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

void ARIPlayerController::MenuBack()
{
    if (MenuMode == ERIMenuMode::Settings)
    {
        ReturnFromSettings();
    }
    else if (MenuMode == ERIMenuMode::Pause)
    {
        ResumeGame();
    }
}

void ARIPlayerController::FinishQuickRestart()
{
    if (MenuMode != ERIMenuMode::None || !IsPlayerRaceFinished()) return;

    UE_LOG(LogTemp, Display, TEXT("RI INPUT FINISH_RESTART source=Y"));
    RestartConfiguredRace();
}

void ARIPlayerController::AdjustSelectedSetting(const int32 Delta)
{
    if (Delta == 0) return;

    if (MenuMode == ERIMenuMode::RaceSetup)
    {
        if (SelectedMenuRow >= 4 || !GetWorld()) return;

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
        else if (SelectedMenuRow == 3)
        {
            Settings->SetChaosLevel(Settings->GetChaosLevel() + Delta);
        }
        return;
    }

    if (MenuMode != ERIMenuMode::Settings || !GetWorld()) return;

    UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    URIRaceSettingsSubsystem* RaceSettings = GameInstance
        ? GameInstance->GetSubsystem<URIRaceSettingsSubsystem>()
        : nullptr;

    if (SelectedMenuRow == 0 && UserSettings)
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
    else if (SelectedMenuRow == 1 && UserSettings)
    {
        UserSettings->SetVSyncEnabled(!UserSettings->IsVSyncEnabled());
        UserSettings->ApplySettings(false);
    }
    else if (SelectedMenuRow == 2 && RaceSettings)
    {
        RaceSettings->SetSteeringFeel(RaceSettings->GetSteeringFeel() + Delta);
    }
}

void ARIPlayerController::MenuConfirm()
{
    if (!GetWorld()) return;

    if (MenuMode == ERIMenuMode::None)
    {
        if (IsPlayerRaceFinished())
        {
            UE_LOG(LogTemp, Display, TEXT("RI INPUT FINISH_RESTART source=CONFIRM"));
            RestartConfiguredRace();
        }
        return;
    }

    if (MenuMode == ERIMenuMode::RaceSetup)
    {
        if (SelectedMenuRow == 4)
        {
            ARIGameMode* GameMode = GetWorld()->GetAuthGameMode<ARIGameMode>();
            if (!GameMode) return;

            MenuMode = ERIMenuMode::None;
            SelectedMenuRow = 0;
            GameMode->StartConfiguredRace();
            SetInputMode(FInputModeGameOnly());
        }
        else if (SelectedMenuRow == 5)
        {
            OpenSettingsMenu();
        }
        else if (SelectedMenuRow == 6)
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

    if (MenuMode == ERIMenuMode::Settings && SelectedMenuRow == 3)
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

    if (MenuMode == ERIMenuMode::None && IsPlayerRaceFinished())
    {
        // The finish panel owns the post-race state. Start/P/Escape should not
        // replace it with the pause menu or hide the race-again action.
        UE_LOG(LogTemp, Display, TEXT("RI INPUT FINISH_LOCK pause=blocked"));
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
