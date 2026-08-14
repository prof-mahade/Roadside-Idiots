#include "Debug/RIDebugHUD.h"
#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Race/RIRaceManager.h"
#include "AI/RIAIController.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

void ARIDebugHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !PlayerOwner) return;

    ARIBikePawn* Bike = Cast<ARIBikePawn>(PlayerOwner->GetPawn());
    if (!Bike) return;

    if (!CachedRaceManager)
    {
        for (TActorIterator<ARIRaceManager> It(GetWorld()); It; ++It)
        {
            CachedRaceManager = *It;
            break;
        }
    }

    UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
    float Y = 28.0f;
    auto Line = [&](const FString& Text, const FLinearColor& Color = FLinearColor::White)
    {
        DrawText(Text, Color, 28.0f, Y, Font, 1.05f, false);
        Y += 24.0f;
    };

    Line(TEXT("ROADSIDE IDIOTS - MVP"), FLinearColor(1.0f, 0.75f, 0.2f));
    Line(TEXT("BUILD: VPR-06 | RIVALS: PERSONALITY | ROAD: SEAMLESS"), FLinearColor(0.55f, 1.0f, 0.70f));
    Line(FString::Printf(TEXT("Speed: %.0f km/h"), FMath::Abs(Bike->GetBikeMovement()->GetForwardSpeedKph())));
    Line(FString::Printf(TEXT("Condition: %.0f / %.0f"), Bike->GetHealthComponent()->GetCurrentHealth(), Bike->GetHealthComponent()->GetMaxHealth()));

    if (CachedRaceManager)
    {
        const FName Id = Bike->GetParticipantComponent()->GetParticipantId();
        FRIRaceProgress Progress;
        if (CachedRaceManager->GetProgress(Id, Progress))
        {
            if (Progress.bFinished)
            {
                Line(FString::Printf(TEXT("FINISHED - Place %d/%d"), CachedRaceManager->GetPlace(Id), CachedRaceManager->GetParticipantCount()), FLinearColor::Green);
                Line(TEXT("Press ENTER for another race."), FLinearColor(1.0f, 0.85f, 0.25f));
            }
            else
            {
                Line(FString::Printf(TEXT("Checkpoint: %d/%d"), Progress.NextCheckpoint, CachedRaceManager->GetCheckpointCount()));
                Line(FString::Printf(TEXT("Place: %d/%d"), CachedRaceManager->GetPlace(Id), CachedRaceManager->GetParticipantCount()));
            }
        }
    }

    int32 AngryRivalCount = 0;
    for (TActorIterator<ARIAIController> It(GetWorld()); It; ++It)
    {
        ARIAIController* AI = *It;
        if (!AI || !AI->IsHoldingGrudgeAgainst(Bike)) continue;

        ARIBikePawn* RivalBike = Cast<ARIBikePawn>(AI->GetPawn());
        const URIParticipantComponent* RivalParticipant = RivalBike ? RivalBike->GetParticipantComponent() : nullptr;
        const FString RivalName = RivalParticipant ? RivalParticipant->GetParticipantId().ToString() : TEXT("RIVAL");

        if (AngryRivalCount == 0)
        {
            Y += 4.0f;
        }

        Line(
            FString::Printf(TEXT("MAD AT YOU: %s [%s] - %.0fs"),
                *RivalName,
                *AI->GetPersonalityLabel(),
                AI->GetGrudgeTimeRemaining()),
            FLinearColor(1.0f, 0.28f, 0.12f));
        ++AngryRivalCount;
    }

    Y += 12.0f;
    Line(TEXT("W accelerate | S brake/reverse | A/D steer"), FLinearColor(0.75f, 0.85f, 1.0f));
    Line(TEXT("Q/E slap left/right | R recover | ENTER restart"), FLinearColor(0.75f, 0.85f, 1.0f));
}
