#include "Debug/RIDebugHUD.h"
#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Race/RIRaceManager.h"
#include "AI/RIAIController.h"
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
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
    Line(TEXT("BUILD: VPR-09 | DAMAGE: BANDAGES | COMBAT: IMPACT FEEL"), FLinearColor(0.55f, 1.0f, 0.70f));
    Line(FString::Printf(TEXT("Speed: %.0f km/h"), FMath::Abs(Bike->GetBikeMovement()->GetForwardSpeedKph())));

    const float CurrentCondition = Bike->GetHealthComponent()->GetCurrentHealth();
    const float MaxCondition = FMath::Max(1.0f, Bike->GetHealthComponent()->GetMaxHealth());
    const float ConditionFraction = CurrentCondition / MaxCondition;

    FLinearColor ConditionColor = FLinearColor::White;
    if (ConditionFraction <= 0.25f)
    {
        ConditionColor = FLinearColor(1.0f, 0.18f, 0.10f);
    }
    else if (ConditionFraction <= 0.50f)
    {
        ConditionColor = FLinearColor(1.0f, 0.48f, 0.10f);
    }
    else if (ConditionFraction <= 0.75f)
    {
        ConditionColor = FLinearColor(1.0f, 0.82f, 0.18f);
    }

    Line(FString::Printf(TEXT("Condition: %.0f / %.0f"), CurrentCondition, MaxCondition), ConditionColor);

    if (ConditionFraction <= 0.25f)
    {
        Line(TEXT("DAMAGE: HELD TOGETHER BY BANDAGES"), FLinearColor(1.0f, 0.18f, 0.10f));
    }
    else if (ConditionFraction <= 0.50f)
    {
        Line(TEXT("DAMAGE: ROUGH SHAPE"), FLinearColor(1.0f, 0.48f, 0.10f));
    }
    else if (ConditionFraction <= 0.75f)
    {
        Line(TEXT("DAMAGE: BANGED UP"), FLinearColor(1.0f, 0.82f, 0.18f));
    }

    Line(FString::Printf(TEXT("Banana peels: %d / 3"), Bike->GetBananaPeelCount()), FLinearColor(1.0f, 0.85f, 0.18f));

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

    FString PlayerImpactText;
    float PlayerImpactAlpha = 0.0f;
    if (Bike->GetActiveComicImpact(PlayerImpactText, PlayerImpactAlpha))
    {
        const FLinearColor ImpactColor(1.0f, 0.16f, 0.06f, PlayerImpactAlpha);
        DrawText(PlayerImpactText, ImpactColor, Canvas->SizeX * 0.46f, Canvas->SizeY * 0.43f, Font, 2.0f, false);
    }

    for (TActorIterator<ARIAIController> It(GetWorld()); It; ++It)
    {
        ARIAIController* AI = *It;
        ARIBikePawn* RivalBike = AI ? Cast<ARIBikePawn>(AI->GetPawn()) : nullptr;
        if (!AI || !RivalBike) continue;

        const float DistanceSq = FVector::DistSquared(Bike->GetActorLocation(), RivalBike->GetActorLocation());
        if (DistanceSq > FMath::Square(6500.0f)) continue;

        FVector2D ScreenPosition;
        const FVector LabelWorldLocation = RivalBike->GetActorLocation() + FVector::UpVector * 255.0f;
        if (!PlayerOwner->ProjectWorldLocationToScreen(LabelWorldLocation, ScreenPosition, true)) continue;
        if (ScreenPosition.X < -100.0f || ScreenPosition.X > Canvas->SizeX + 100.0f ||
            ScreenPosition.Y < -60.0f || ScreenPosition.Y > Canvas->SizeY + 60.0f)
        {
            continue;
        }

        const URIParticipantComponent* RivalParticipant = RivalBike->GetParticipantComponent();
        const FString RivalName = RivalParticipant ? RivalParticipant->GetParticipantId().ToString() : TEXT("RIVAL");
        const bool bMad = AI->IsHoldingGrudgeAgainst(Bike);

        FString Label = FString::Printf(
            TEXT("%s [%s]"),
            *RivalName,
            *AI->GetPersonalityLabel());
        if (bMad)
        {
            Label += TEXT(" !! MAD !!");
        }

        FLinearColor LabelColor(0.75f, 0.90f, 1.0f);
        if (bMad)
        {
            LabelColor = FLinearColor(1.0f, 0.18f, 0.08f);
        }
        else if (AI->GetPersonalityLabel().Equals(TEXT("LEECH"), ESearchCase::IgnoreCase))
        {
            LabelColor = FLinearColor(0.30f, 1.0f, 0.72f);
        }
        else if (AI->GetPersonalityLabel().Equals(TEXT("HOTHEAD"), ESearchCase::IgnoreCase))
        {
            LabelColor = FLinearColor(1.0f, 0.55f, 0.12f);
        }
        else if (AI->GetPersonalityLabel().Equals(TEXT("PETTY"), ESearchCase::IgnoreCase))
        {
            LabelColor = FLinearColor(0.86f, 0.52f, 1.0f);
        }

        DrawText(Label, LabelColor, ScreenPosition.X - 72.0f, ScreenPosition.Y, Font, 0.88f, false);

        FString RivalImpactText;
        float RivalImpactAlpha = 0.0f;
        if (RivalBike->GetActiveComicImpact(RivalImpactText, RivalImpactAlpha))
        {
            const float PopScale = 1.1f + RivalImpactAlpha * 0.55f;
            const FLinearColor PopColor(1.0f, 0.88f, 0.05f, RivalImpactAlpha);
            DrawText(RivalImpactText, PopColor, ScreenPosition.X - 38.0f, ScreenPosition.Y - 34.0f, Font, PopScale, false);
        }
    }

    int32 AngryRivalCount = 0;
    for (TActorIterator<ARIAIController> It(GetWorld()); It; ++It)
    {
        ARIAIController* AI = *It;
        if (!AI || !AI->IsHoldingGrudgeAgainst(Bike)) continue;

        ARIBikePawn* RivalBike = Cast<ARIBikePawn>(AI->GetPawn());
        if (!RivalBike) continue;

        const URIParticipantComponent* RivalParticipant = RivalBike->GetParticipantComponent();
        const FString RivalName = RivalParticipant ? RivalParticipant->GetParticipantId().ToString() : TEXT("RIVAL");

        FVector ToRival = RivalBike->GetActorLocation() - Bike->GetActorLocation();
        ToRival.Z = 0.0f;
        const float DistanceMeters = ToRival.Size() / 100.0f;
        const FVector RivalDirection = ToRival.GetSafeNormal();
        const float ForwardDot = FVector::DotProduct(RivalDirection, Bike->GetActorForwardVector().GetSafeNormal2D());
        const float RightDot = FVector::DotProduct(RivalDirection, Bike->GetActorRightVector().GetSafeNormal2D());

        FString RelativeDirection;
        if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
        {
            RelativeDirection = ForwardDot >= 0.0f ? TEXT("AHEAD") : TEXT("BEHIND");
        }
        else
        {
            RelativeDirection = RightDot >= 0.0f ? TEXT("RIGHT") : TEXT("LEFT");
        }

        if (AngryRivalCount == 0)
        {
            Y += 4.0f;
        }

        Line(
            FString::Printf(TEXT("MAD: %s [%s] | %.0fs | %s %.0fm"),
                *RivalName,
                *AI->GetPersonalityLabel(),
                AI->GetGrudgeTimeRemaining(),
                *RelativeDirection,
                DistanceMeters),
            FLinearColor(1.0f, 0.28f, 0.12f));
        ++AngryRivalCount;
    }

    Y += 12.0f;
    Line(TEXT("W accelerate | S brake/reverse | A/D steer"), FLinearColor(0.75f, 0.85f, 1.0f));
    Line(TEXT("Q/E slap left/right | F drop peel | R recover | ENTER restart"), FLinearColor(0.75f, 0.85f, 1.0f));
}
