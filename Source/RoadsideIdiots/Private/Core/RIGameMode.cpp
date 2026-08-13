#include "Core/RIGameMode.h"
#include "Race/RIRaceManager.h"
#include "World/RIDemoWorldBuilder.h"
#include "Debug/RIDebugHUD.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ARIGameMode::ARIGameMode()
{
    DefaultPawnClass = nullptr;
    PlayerControllerClass = APlayerController::StaticClass();
    HUDClass = ARIDebugHUD::StaticClass();
}

void ARIGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (!HasAuthority() || !GetWorld()) return;

    RaceManager = GetWorld()->SpawnActor<ARIRaceManager>();
    DemoWorldBuilder = GetWorld()->SpawnActor<ARIDemoWorldBuilder>();
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

    if (RaceManager && DemoWorldBuilder)
    {
        DemoWorldBuilder->BuildWorld(RaceManager, PlayerController);
    }
}
