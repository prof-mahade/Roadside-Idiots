#include "Core/RIPlayerController.h"

#include "Core/RIGameMode.h"
#include "Core/RIRaceSettingsSubsystem.h"
#include "Components/InputComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
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

    // Epic documents BindKey's returned reference as valid only until another
    // key is bound, so set bConsumeInput immediately on each new binding.
    InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &ARIPlayerController::SetupPrevious).bConsumeInput = false;
    InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &ARIPlayerController::SetupNext).bConsumeInput = false;
    InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &ARIPlayerController::SetupDecrease).bConsumeInput = false;
    InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &ARIPlayerController::SetupIncrease).bConsumeInput = false;
    InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ARIPlayerController::SetupConfirm).bConsumeInput = false;
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
