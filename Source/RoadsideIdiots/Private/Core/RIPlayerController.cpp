#include "Core/RIPlayerController.h"

#include "Core/RIGameMode.h"
#include "Core/RIRaceSettingsSubsystem.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"

ARIPlayerController::ARIPlayerController()
{
    bShowMouseCursor = false;
}

void ARIPlayerController::BeginPlay()
{
    Super::BeginPlay();
    bRaceSetupMenuOpen = true;
    SelectedSetupRow = 0;
    SetInputMode(FInputModeGameOnly());
}

void ARIPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputComponent) return;

    InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &ARIPlayerController::SetupPrevious);
    InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &ARIPlayerController::SetupNext);
    InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &ARIPlayerController::SetupDecrease);
    InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &ARIPlayerController::SetupIncrease);
    InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ARIPlayerController::SetupConfirm);
}

void ARIPlayerController::SetupPrevious()
{
    if (!bRaceSetupMenuOpen) return;
    SelectedSetupRow = (SelectedSetupRow + 3) % 4;
}

void ARIPlayerController::SetupNext()
{
    if (!bRaceSetupMenuOpen) return;
    SelectedSetupRow = (SelectedSetupRow + 1) % 4;
}

void ARIPlayerController::SetupDecrease()
{
    AdjustSelectedSetting(-1);
}

void ARIPlayerController::SetupIncrease()
{
    AdjustSelectedSetting(1);
}

void ARIPlayerController::AdjustSelectedSetting(const int32 Delta)
{
    if (!bRaceSetupMenuOpen || SelectedSetupRow >= 3 || !GetWorld()) return;

    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    URIRaceSettingsSubsystem* Settings = GameInstance
        ? GameInstance->GetSubsystem<URIRaceSettingsSubsystem>()
        : nullptr;
    if (!Settings) return;

    if (SelectedSetupRow == 0)
    {
        Settings->SetOpponentCount(Settings->GetOpponentCount() + Delta);
    }
    else if (SelectedSetupRow == 1)
    {
        Settings->SetLapCount(Settings->GetLapCount() + Delta);
    }
    else if (SelectedSetupRow == 2)
    {
        Settings->SetTrafficCount(Settings->GetTrafficCount() + Delta);
    }
}

void ARIPlayerController::SetupConfirm()
{
    if (!bRaceSetupMenuOpen || SelectedSetupRow != 3 || !GetWorld()) return;

    ARIGameMode* GameMode = GetWorld()->GetAuthGameMode<ARIGameMode>();
    if (!GameMode) return;

    bRaceSetupMenuOpen = false;
    GameMode->StartConfiguredRace();
    SetInputMode(FInputModeGameOnly());
}
