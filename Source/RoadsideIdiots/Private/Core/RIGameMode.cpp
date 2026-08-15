#include "Core/RIGameMode.h"
#include "Core/RIPlayerController.h"
#include "Race/RIRaceManager.h"
#include "World/RIDemoWorldBuilder.h"
#include "Debug/RIRaceSetupHUD.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ARIGameMode::ARIGameMode()
{
    DefaultPawnClass = nullptr;
    PlayerControllerClass = ARIPlayerController::StaticClass();
    HUDClass = ARIRaceSetupHUD::StaticClass();
}

void ARIGameMode::BeginPlay()
{
    Super::BeginPlay();

    // VPR-23A intentionally waits for the local race-setup menu. The world is
    // built only after the player confirms opponents/laps/traffic.
}

void ARIGameMode::StartConfiguredRace()
{
    if (bConfiguredRaceStarted || !HasAuthority() || !GetWorld()) return;

    RaceManager = GetWorld()->SpawnActor<ARIRaceManager>();
    DemoWorldBuilder = GetWorld()->SpawnActor<ARIDemoWorldBuilder>();
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

    if (RaceManager && DemoWorldBuilder)
    {
        bConfiguredRaceStarted = true;
        DemoWorldBuilder->BuildWorld(RaceManager, PlayerController);
    }
}
