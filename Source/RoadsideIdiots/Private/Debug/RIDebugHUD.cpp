#include "Debug/RIDebugHUD.h"
#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Race/RIRaceManager.h"
#include "AI/RIAIController.h"
#include "Items/RIRottenEggStinkEffect.h"
#include "Traffic/RITrafficVehicle.h"
#include "Hazards/RIPoopWorldSubsystem.h"
#include "Hazards/RIPoopMessEffect.h"
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

    const FName PlayerId = Bike->GetParticipantComponent()->GetParticipantId();
    FRIRaceProgress PlayerProgress;
    const bool bHasRaceProgress = CachedRaceManager && CachedRaceManager->GetProgress(PlayerId, PlayerProgress);
    const int32 TotalLaps = CachedRaceManager ? CachedRaceManager->GetTotalLaps() : 1;
    const int32 PlayerPlace = CachedRaceManager ? CachedRaceManager->GetPlace(PlayerId) : 1;
    const int32 ParticipantCount = CachedRaceManager ? CachedRaceManager->GetParticipantCount() : 1;
    const int32 CurrentLap = bHasRaceProgress
        ? FMath::Clamp(PlayerProgress.bFinished ? TotalLaps : PlayerProgress.CompletedLaps + 1, 1, TotalLaps)
        : 1;
    const float RaceTime = CachedRaceManager
        ? (PlayerProgress.bFinished ? PlayerProgress.FinishTime : CachedRaceManager->GetRaceElapsedTime())
        : 0.0f;

    const int32 RaceMinutes = FMath::FloorToInt(RaceTime / 60.0f);
    const float RaceSeconds = RaceTime - static_cast<float>(RaceMinutes * 60);
    const FString RaceTimeText = FString::Printf(TEXT("%d:%05.2f"), RaceMinutes, RaceSeconds);

    Line(TEXT("ROADSIDE IDIOTS - MVP"), FLinearColor(1.0f, 0.75f, 0.2f));
    Line(TEXT("BUILD: VPR-14 | MINIMAP + 3 LAPS | AI: OPTIMIZED"), FLinearColor(0.55f, 1.0f, 0.70f));
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
    Line(
        FString::Printf(TEXT("Rotten eggs: %d / %d | G throw"), Bike->GetRottenEggCount(), Bike->GetMaxRottenEggs()),
        FLinearColor(0.55f, 0.78f, 0.12f));

    TArray<ARITrafficVehicle*> TrafficVehicles;
    for (TActorIterator<ARITrafficVehicle> It(GetWorld()); It; ++It)
    {
        if (*It)
        {
            TrafficVehicles.Add(*It);
        }
    }
    Line(FString::Printf(TEXT("Traffic: %d civilian idiots"), TrafficVehicles.Num()), FLinearColor(0.72f, 0.82f, 1.0f));

    if (const URIPoopWorldSubsystem* PoopSystem = GetWorld()->GetSubsystem<URIPoopWorldSubsystem>())
    {
        Line(
            FString::Printf(TEXT("Road hazards: %d dog poop | %d cow patties"),
                PoopSystem->GetSpawnedDogPoopCount(),
                PoopSystem->GetSpawnedCowPoopCount()),
            FLinearColor(0.67f, 0.43f, 0.20f));
    }

    if (bHasRaceProgress && CachedRaceManager)
    {
        if (PlayerProgress.bFinished)
        {
            Line(FString::Printf(TEXT("FINISHED - Place %d/%d | Time %s"), PlayerPlace, ParticipantCount, *RaceTimeText), FLinearColor::Green);
            Line(TEXT("Press ENTER for another race."), FLinearColor(1.0f, 0.85f, 0.25f));
        }
        else
        {
            Line(FString::Printf(TEXT("Lap: %d/%d | Checkpoint: %d/%d"),
                CurrentLap,
                TotalLaps,
                PlayerProgress.NextCheckpoint,
                CachedRaceManager->GetCheckpointCount()));
            Line(FString::Printf(TEXT("Place: %d/%d | Time: %s"), PlayerPlace, ParticipantCount, *RaceTimeText));
        }
    }

    // Compact race strip near the top center. It remains useful after the left
    // debug block is eventually replaced by the final HUD.
    if (CachedRaceManager && bHasRaceProgress)
    {
        const FString RaceStrip = FString::Printf(
            TEXT("LAP %d/%d     POS %d/%d     %s"),
            CurrentLap,
            TotalLaps,
            PlayerPlace,
            ParticipantCount,
            *RaceTimeText);
        DrawText(
            RaceStrip,
            FLinearColor(0.95f, 0.97f, 1.0f),
            Canvas->SizeX * 0.39f,
            28.0f,
            Font,
            1.35f,
            false);
    }

    // Real 3-2-1-GO countdown. Bike/AI drive inputs are gated by the same race
    // manager state, so this is not just a decorative overlay.
    if (CachedRaceManager)
    {
        const float SecondsUntilStart = CachedRaceManager->GetSecondsUntilStart();
        if (SecondsUntilStart > 0.0f)
        {
            const int32 Count = FMath::Max(1, FMath::CeilToInt(SecondsUntilStart));
            DrawText(
                FString::FromInt(Count),
                FLinearColor(1.0f, 0.72f, 0.08f),
                Canvas->SizeX * 0.49f,
                Canvas->SizeY * 0.27f,
                Font,
                4.0f,
                false);
        }
        else if (CachedRaceManager->GetRaceElapsedTime() < 0.85f)
        {
            DrawText(
                TEXT("GO!"),
                FLinearColor(0.20f, 1.0f, 0.28f),
                Canvas->SizeX * 0.46f,
                Canvas->SizeY * 0.27f,
                Font,
                3.6f,
                false);
        }
    }

    TArray<ARIAIController*> RivalControllers;
    for (TActorIterator<ARIAIController> It(GetWorld()); It; ++It)
    {
        if (*It)
        {
            RivalControllers.Add(*It);
        }
    }

    // Circular minimap in the top-right. The prototype oval is normalized by
    // its X/Y radii, so the course becomes a clean round racing-game map while
    // still placing all actors according to their real world position.
    {
        constexpr float MapWorldRadiusX = 9000.0f;
        constexpr float MapWorldRadiusY = 5000.0f;
        constexpr int32 CircleSegments = 48;
        const FVector2D MapCenter(Canvas->SizeX - 155.0f, 165.0f);
        const float OuterRadius = 112.0f;
        const float TrackRadius = 86.0f;

        auto CirclePoint = [&](float Radius, int32 Index)
        {
            const float Angle = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(CircleSegments);
            return MapCenter + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;
        };

        auto DrawCircle = [&](float Radius, const FLinearColor& Color, float Thickness)
        {
            for (int32 Index = 0; Index < CircleSegments; ++Index)
            {
                const FVector2D A = CirclePoint(Radius, Index);
                const FVector2D B = CirclePoint(Radius, (Index + 1) % CircleSegments);
                DrawLine(A.X, A.Y, B.X, B.Y, Color, Thickness);
            }
        };

        auto WorldToMap = [&](const FVector& WorldLocation)
        {
            FVector2D Normalized(
                WorldLocation.X / MapWorldRadiusX,
                -WorldLocation.Y / MapWorldRadiusY);
            const float Length = Normalized.Size();
            if (Length > 1.16f)
            {
                Normalized *= 1.16f / Length;
            }
            return MapCenter + Normalized * TrackRadius;
        };

        auto DrawMarker = [&](const FVector2D& Position, const FLinearColor& Color, float Size, float Thickness)
        {
            DrawLine(Position.X - Size, Position.Y, Position.X + Size, Position.Y, Color, Thickness);
            DrawLine(Position.X, Position.Y - Size, Position.X, Position.Y + Size, Color, Thickness);
        };

        DrawCircle(OuterRadius, FLinearColor(0.25f, 0.30f, 0.36f, 0.95f), 2.2f);
        DrawCircle(TrackRadius - 7.0f, FLinearColor(0.28f, 0.32f, 0.36f, 0.90f), 2.0f);
        DrawCircle(TrackRadius + 7.0f, FLinearColor(0.72f, 0.78f, 0.84f, 0.95f), 2.4f);

        DrawText(
            FString::Printf(TEXT("LAP %d/%d"), CurrentLap, TotalLaps),
            FLinearColor(0.95f, 0.97f, 1.0f),
            MapCenter.X - 36.0f,
            MapCenter.Y - OuterRadius - 24.0f,
            Font,
            1.05f,
            false);

        // Start/finish tick on the right edge of the round course.
        DrawLine(
            MapCenter.X + TrackRadius - 8.0f,
            MapCenter.Y - 10.0f,
            MapCenter.X + TrackRadius + 8.0f,
            MapCenter.Y + 10.0f,
            FLinearColor(1.0f, 0.85f, 0.18f),
            3.0f);

        for (ARITrafficVehicle* Traffic : TrafficVehicles)
        {
            if (!Traffic) continue;
            DrawMarker(WorldToMap(Traffic->GetActorLocation()), FLinearColor(0.72f, 0.76f, 0.80f), 2.5f, 1.4f);
        }

        for (ARIAIController* AI : RivalControllers)
        {
            ARIBikePawn* RivalBike = AI ? Cast<ARIBikePawn>(AI->GetPawn()) : nullptr;
            if (!AI || !RivalBike) continue;

            FLinearColor MarkerColor(0.35f, 0.85f, 1.0f);
            if (AI->IsHoldingGrudgeAgainst(Bike))
            {
                MarkerColor = FLinearColor(1.0f, 0.18f, 0.08f);
            }
            else if (AI->GetPersonalityLabel().Equals(TEXT("HOTHEAD"), ESearchCase::IgnoreCase))
            {
                MarkerColor = FLinearColor(1.0f, 0.55f, 0.12f);
            }
            else if (AI->GetPersonalityLabel().Equals(TEXT("PETTY"), ESearchCase::IgnoreCase))
            {
                MarkerColor = FLinearColor(0.86f, 0.52f, 1.0f);
            }

            DrawMarker(WorldToMap(RivalBike->GetActorLocation()), MarkerColor, 4.0f, 2.0f);
        }

        const FVector2D PlayerMapPosition = WorldToMap(Bike->GetActorLocation());
        DrawMarker(PlayerMapPosition, FLinearColor(1.0f, 0.86f, 0.08f), 5.5f, 2.8f);

        FVector2D Heading(
            Bike->GetActorForwardVector().X / MapWorldRadiusX,
            -Bike->GetActorForwardVector().Y / MapWorldRadiusY);
        if (!Heading.IsNearlyZero())
        {
            Heading.Normalize();
            const FVector2D Tip = PlayerMapPosition + Heading * 13.0f;
            DrawLine(
                PlayerMapPosition.X,
                PlayerMapPosition.Y,
                Tip.X,
                Tip.Y,
                FLinearColor(1.0f, 0.86f, 0.08f),
                2.5f);
        }
    }

    FString PlayerImpactText;
    float PlayerImpactAlpha = 0.0f;
    if (Bike->GetActiveComicImpact(PlayerImpactText, PlayerImpactAlpha))
    {
        const FLinearColor ImpactColor(1.0f, 0.16f, 0.06f, PlayerImpactAlpha);
        DrawText(PlayerImpactText, ImpactColor, Canvas->SizeX * 0.46f, Canvas->SizeY * 0.43f, Font, 2.0f, false);
    }

    for (ARIAIController* AI : RivalControllers)
    {
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
            TEXT("%s [%s] | P%d E%d"),
            *RivalName,
            *AI->GetPersonalityLabel(),
            RivalBike->GetBananaPeelCount(),
            RivalBike->GetRottenEggCount());
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

        DrawText(Label, LabelColor, ScreenPosition.X - 92.0f, ScreenPosition.Y, Font, 0.84f, false);

        FString RivalImpactText;
        float RivalImpactAlpha = 0.0f;
        if (RivalBike->GetActiveComicImpact(RivalImpactText, RivalImpactAlpha))
        {
            const float PopScale = 1.1f + RivalImpactAlpha * 0.55f;
            const FLinearColor PopColor(1.0f, 0.88f, 0.05f, RivalImpactAlpha);
            DrawText(RivalImpactText, PopColor, ScreenPosition.X - 38.0f, ScreenPosition.Y - 34.0f, Font, PopScale, false);
        }
    }

    for (TActorIterator<ARIRottenEggStinkEffect> It(GetWorld()); It; ++It)
    {
        ARIRottenEggStinkEffect* Stink = *It;
        ARIBikePawn* EggedBike = Stink ? Cast<ARIBikePawn>(Stink->GetOwner()) : nullptr;
        if (!EggedBike) continue;

        FVector2D ScreenPosition;
        const FVector LabelLocation = EggedBike->GetActorLocation() + FVector::UpVector * 315.0f;
        if (!PlayerOwner->ProjectWorldLocationToScreen(LabelLocation, ScreenPosition, true)) continue;

        DrawText(
            TEXT("STINK!"),
            FLinearColor(0.48f, 0.88f, 0.06f),
            ScreenPosition.X - 38.0f,
            ScreenPosition.Y - 10.0f,
            Font,
            1.35f,
            false);
    }

    for (TActorIterator<ARIPoopMessEffect> It(GetWorld()); It; ++It)
    {
        ARIPoopMessEffect* Mess = *It;
        ARIBikePawn* DirtyBike = Mess ? Mess->GetAffectedBike() : nullptr;
        if (!DirtyBike) continue;

        FVector2D ScreenPosition;
        const FVector LabelLocation = DirtyBike->GetActorLocation() + FVector::UpVector * 335.0f;
        if (!PlayerOwner->ProjectWorldLocationToScreen(LabelLocation, ScreenPosition, true)) continue;

        DrawText(
            Mess->IsCowMess() ? TEXT("COW STINK!") : TEXT("DOG STINK!"),
            Mess->IsCowMess() ? FLinearColor(0.62f, 0.48f, 0.05f) : FLinearColor(0.48f, 0.62f, 0.08f),
            ScreenPosition.X - 50.0f,
            ScreenPosition.Y - 8.0f,
            Font,
            1.28f,
            false);
    }

    int32 AngryRivalCount = 0;
    for (ARIAIController* AI : RivalControllers)
    {
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

    if (bHasRaceProgress && PlayerProgress.bFinished)
    {
        DrawText(
            FString::Printf(TEXT("FINISH!  PLACE %d/%d"), PlayerPlace, ParticipantCount),
            FLinearColor(0.20f, 1.0f, 0.32f),
            Canvas->SizeX * 0.39f,
            Canvas->SizeY * 0.34f,
            Font,
            2.3f,
            false);
        DrawText(
            FString::Printf(TEXT("TIME %s   -   ENTER TO RACE AGAIN"), *RaceTimeText),
            FLinearColor(1.0f, 0.86f, 0.22f),
            Canvas->SizeX * 0.37f,
            Canvas->SizeY * 0.40f,
            Font,
            1.35f,
            false);
    }

    Y += 12.0f;
    Line(TEXT("W accelerate | S brake/reverse | A/D steer"), FLinearColor(0.75f, 0.85f, 1.0f));
    Line(TEXT("Q/E slap | F drop peel | G throw egg | R recover | ENTER restart"), FLinearColor(0.75f, 0.85f, 1.0f));
}
