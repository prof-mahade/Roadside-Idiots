#include "Debug/RIDebugHUD.h"
#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Race/RIRaceManager.h"
#include "AI/RIAIController.h"
#include "Items/RIRottenEggStinkEffect.h"
#include "Traffic/RITrafficVehicle.h"
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
    if (!Font) return;

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

    const float CurrentCondition = Bike->GetHealthComponent()->GetCurrentHealth();
    const float MaxCondition = FMath::Max(1.0f, Bike->GetHealthComponent()->GetMaxHealth());
    const float ConditionFraction = CurrentCondition / MaxCondition;

    FLinearColor ConditionColor = FLinearColor::White;
    FString DamageText;
    if (ConditionFraction <= 0.25f)
    {
        ConditionColor = FLinearColor(1.0f, 0.18f, 0.10f);
        DamageText = TEXT("HELD TOGETHER BY BANDAGES");
    }
    else if (ConditionFraction <= 0.50f)
    {
        ConditionColor = FLinearColor(1.0f, 0.48f, 0.10f);
        DamageText = TEXT("ROUGH SHAPE");
    }
    else if (ConditionFraction <= 0.75f)
    {
        ConditionColor = FLinearColor(1.0f, 0.82f, 0.18f);
        DamageText = TEXT("BANGED UP");
    }

    TArray<ARITrafficVehicle*> TrafficVehicles;
    for (TActorIterator<ARITrafficVehicle> It(GetWorld()); It; ++It)
    {
        if (*It) TrafficVehicles.Add(*It);
    }

    TArray<ARIAIController*> RivalControllers;
    for (TActorIterator<ARIAIController> It(GetWorld()); It; ++It)
    {
        if (*It) RivalControllers.Add(*It);
    }

    // Compact left HUD: keep gameplay information readable and stop turning the
    // screen into a debug console while we are validating presentation.
    DrawRect(FLinearColor(0.015f, 0.022f, 0.030f, 0.58f), 16.0f, 18.0f, 330.0f, DamageText.IsEmpty() ? 142.0f : 164.0f);

    float LeftY = 28.0f;
    auto LeftLine = [&](const FString& Text, const FLinearColor& Color = FLinearColor::White, float Scale = 0.92f)
    {
        DrawText(Text, Color, 28.0f, LeftY, Font, Scale, false);
        LeftY += 20.0f;
    };

    LeftLine(TEXT("ROADSIDE IDIOTS"), FLinearColor(1.0f, 0.76f, 0.18f), 1.02f);
    LeftLine(TEXT("VPR-14.1 | HUD CLEANUP | PACK SPACING"), FLinearColor(0.52f, 1.0f, 0.70f), 0.78f);
    LeftLine(FString::Printf(TEXT("SPEED  %.0f km/h"), FMath::Abs(Bike->GetBikeMovement()->GetForwardSpeedKph())));
    LeftLine(FString::Printf(TEXT("CONDITION  %.0f / %.0f"), CurrentCondition, MaxCondition), ConditionColor);
    LeftLine(
        FString::Printf(TEXT("PEELS %d/3     EGGS %d/%d"), Bike->GetBananaPeelCount(), Bike->GetRottenEggCount(), Bike->GetMaxRottenEggs()),
        FLinearColor(0.95f, 0.86f, 0.28f),
        0.88f);
    if (!DamageText.IsEmpty())
    {
        LeftLine(FString::Printf(TEXT("DAMAGE  %s"), *DamageText), ConditionColor, 0.80f);
    }

    // Race strip gets its own dark backing so sky/filth effects cannot wash it out.
    if (CachedRaceManager && bHasRaceProgress)
    {
        const float StripX = Canvas->SizeX * 0.37f;
        const float StripW = 390.0f;
        DrawRect(FLinearColor(0.015f, 0.022f, 0.030f, 0.62f), StripX - 15.0f, 17.0f, StripW, 38.0f);
        DrawText(
            FString::Printf(TEXT("LAP %d/%d     POS %d/%d     %s"), CurrentLap, TotalLaps, PlayerPlace, ParticipantCount, *RaceTimeText),
            FLinearColor(0.96f, 0.98f, 1.0f),
            StripX,
            27.0f,
            Font,
            1.25f,
            false);
    }

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

    // Circular top-right minimap. Keep the map itself bright, but give it a
    // subtle dark panel so markers remain visible against the sky.
    {
        constexpr float MapWorldRadiusX = 9000.0f;
        constexpr float MapWorldRadiusY = 5000.0f;
        constexpr int32 CircleSegments = 48;
        const FVector2D MapCenter(Canvas->SizeX - 145.0f, 150.0f);
        const float OuterRadius = 105.0f;
        const float TrackRadius = 80.0f;

        DrawRect(
            FLinearColor(0.015f, 0.022f, 0.030f, 0.55f),
            MapCenter.X - OuterRadius - 12.0f,
            MapCenter.Y - OuterRadius - 30.0f,
            (OuterRadius + 12.0f) * 2.0f,
            (OuterRadius + 12.0f) * 2.0f + 30.0f);

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
            FVector2D Normalized(WorldLocation.X / MapWorldRadiusX, -WorldLocation.Y / MapWorldRadiusY);
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

        DrawText(TEXT("MAP"), FLinearColor(0.90f, 0.94f, 1.0f), MapCenter.X - 16.0f, MapCenter.Y - OuterRadius - 22.0f, Font, 0.90f, false);
        DrawCircle(OuterRadius, FLinearColor(0.34f, 0.39f, 0.45f, 0.95f), 2.0f);
        DrawCircle(TrackRadius - 6.0f, FLinearColor(0.30f, 0.34f, 0.39f, 0.95f), 1.8f);
        DrawCircle(TrackRadius + 6.0f, FLinearColor(0.76f, 0.82f, 0.88f, 0.98f), 2.3f);

        DrawLine(
            MapCenter.X + TrackRadius - 7.0f,
            MapCenter.Y - 9.0f,
            MapCenter.X + TrackRadius + 7.0f,
            MapCenter.Y + 9.0f,
            FLinearColor(1.0f, 0.85f, 0.18f),
            3.0f);

        for (ARITrafficVehicle* Traffic : TrafficVehicles)
        {
            if (Traffic)
            {
                DrawMarker(WorldToMap(Traffic->GetActorLocation()), FLinearColor(0.66f, 0.70f, 0.74f), 2.2f, 1.3f);
            }
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

            DrawMarker(WorldToMap(RivalBike->GetActorLocation()), MarkerColor, 3.8f, 2.0f);
        }

        const FVector2D PlayerMapPosition = WorldToMap(Bike->GetActorLocation());
        DrawMarker(PlayerMapPosition, FLinearColor(1.0f, 0.86f, 0.08f), 5.2f, 2.8f);

        FVector2D Heading(Bike->GetActorForwardVector().X / MapWorldRadiusX, -Bike->GetActorForwardVector().Y / MapWorldRadiusY);
        if (!Heading.IsNearlyZero())
        {
            Heading.Normalize();
            const FVector2D Tip = PlayerMapPosition + Heading * 12.0f;
            DrawLine(PlayerMapPosition.X, PlayerMapPosition.Y, Tip.X, Tip.Y, FLinearColor(1.0f, 0.86f, 0.08f), 2.5f);
        }
    }

    FString PlayerImpactText;
    float PlayerImpactAlpha = 0.0f;
    if (Bike->GetActiveComicImpact(PlayerImpactText, PlayerImpactAlpha))
    {
        const FLinearColor ImpactColor(1.0f, 0.20f, 0.07f, PlayerImpactAlpha);
        DrawText(PlayerImpactText, ImpactColor, Canvas->SizeX * 0.45f, Canvas->SizeY * 0.41f, Font, 1.9f, false);
    }

    // Only show rival world labels when they are close enough to matter, or when
    // they are actively angry at the player. This keeps the road readable.
    for (ARIAIController* AI : RivalControllers)
    {
        ARIBikePawn* RivalBike = AI ? Cast<ARIBikePawn>(AI->GetPawn()) : nullptr;
        if (!AI || !RivalBike) continue;

        const bool bMad = AI->IsHoldingGrudgeAgainst(Bike);
        const float DistanceSq = FVector::DistSquared2D(Bike->GetActorLocation(), RivalBike->GetActorLocation());
        if (!bMad && DistanceSq > FMath::Square(3000.0f)) continue;
        if (bMad && DistanceSq > FMath::Square(5200.0f)) continue;

        FVector2D ScreenPosition;
        const FVector LabelWorldLocation = RivalBike->GetActorLocation() + FVector::UpVector * 245.0f;
        if (!PlayerOwner->ProjectWorldLocationToScreen(LabelWorldLocation, ScreenPosition, true)) continue;
        if (ScreenPosition.X < -80.0f || ScreenPosition.X > Canvas->SizeX + 80.0f ||
            ScreenPosition.Y < -50.0f || ScreenPosition.Y > Canvas->SizeY + 50.0f)
        {
            continue;
        }

        const URIParticipantComponent* RivalParticipant = RivalBike->GetParticipantComponent();
        const FString RivalName = RivalParticipant ? RivalParticipant->GetParticipantId().ToString() : TEXT("RIVAL");

        FString Label = FString::Printf(
            TEXT("%s [%s]  P%d E%d"),
            *RivalName,
            *AI->GetPersonalityLabel(),
            RivalBike->GetBananaPeelCount(),
            RivalBike->GetRottenEggCount());
        if (bMad) Label += TEXT("  MAD!");

        FLinearColor LabelColor(0.74f, 0.90f, 1.0f);
        if (bMad) LabelColor = FLinearColor(1.0f, 0.20f, 0.08f);
        else if (AI->GetPersonalityLabel().Equals(TEXT("LEECH"), ESearchCase::IgnoreCase)) LabelColor = FLinearColor(0.30f, 1.0f, 0.72f);
        else if (AI->GetPersonalityLabel().Equals(TEXT("HOTHEAD"), ESearchCase::IgnoreCase)) LabelColor = FLinearColor(1.0f, 0.55f, 0.12f);
        else if (AI->GetPersonalityLabel().Equals(TEXT("PETTY"), ESearchCase::IgnoreCase)) LabelColor = FLinearColor(0.86f, 0.52f, 1.0f);

        DrawText(Label, LabelColor, ScreenPosition.X - 72.0f, ScreenPosition.Y, Font, 0.72f, false);

        FString RivalImpactText;
        float RivalImpactAlpha = 0.0f;
        if (RivalBike->GetActiveComicImpact(RivalImpactText, RivalImpactAlpha))
        {
            DrawText(
                RivalImpactText,
                FLinearColor(1.0f, 0.88f, 0.05f, RivalImpactAlpha),
                ScreenPosition.X - 30.0f,
                ScreenPosition.Y - 28.0f,
                Font,
                1.0f + RivalImpactAlpha * 0.40f,
                false);
        }
    }

    // Stink labels are useful, but smaller and range-limited so they do not
    // dominate the entire sky when the pack is close together.
    for (TActorIterator<ARIRottenEggStinkEffect> It(GetWorld()); It; ++It)
    {
        ARIRottenEggStinkEffect* Stink = *It;
        ARIBikePawn* EggedBike = Stink ? Cast<ARIBikePawn>(Stink->GetOwner()) : nullptr;
        if (!EggedBike || FVector::DistSquared2D(Bike->GetActorLocation(), EggedBike->GetActorLocation()) > FMath::Square(3400.0f)) continue;

        FVector2D ScreenPosition;
        if (!PlayerOwner->ProjectWorldLocationToScreen(EggedBike->GetActorLocation() + FVector::UpVector * 290.0f, ScreenPosition, true)) continue;
        DrawText(TEXT("STINK!"), FLinearColor(0.48f, 0.88f, 0.06f), ScreenPosition.X - 28.0f, ScreenPosition.Y, Font, 0.92f, false);
    }

    for (TActorIterator<ARIPoopMessEffect> It(GetWorld()); It; ++It)
    {
        ARIPoopMessEffect* Mess = *It;
        ARIBikePawn* DirtyBike = Mess ? Mess->GetAffectedBike() : nullptr;
        if (!DirtyBike || FVector::DistSquared2D(Bike->GetActorLocation(), DirtyBike->GetActorLocation()) > FMath::Square(3400.0f)) continue;

        FVector2D ScreenPosition;
        if (!PlayerOwner->ProjectWorldLocationToScreen(DirtyBike->GetActorLocation() + FVector::UpVector * 300.0f, ScreenPosition, true)) continue;
        DrawText(
            Mess->IsCowMess() ? TEXT("COW STINK") : TEXT("DOG STINK"),
            Mess->IsCowMess() ? FLinearColor(0.65f, 0.50f, 0.08f) : FLinearColor(0.48f, 0.66f, 0.08f),
            ScreenPosition.X - 38.0f,
            ScreenPosition.Y,
            Font,
            0.88f,
            false);
    }

    // Show one concise angry-rival warning instead of a growing debug list.
    for (ARIAIController* AI : RivalControllers)
    {
        if (!AI || !AI->IsHoldingGrudgeAgainst(Bike)) continue;
        ARIBikePawn* RivalBike = Cast<ARIBikePawn>(AI->GetPawn());
        if (!RivalBike) continue;

        const URIParticipantComponent* RivalParticipant = RivalBike->GetParticipantComponent();
        const FString RivalName = RivalParticipant ? RivalParticipant->GetParticipantId().ToString() : TEXT("RIVAL");
        const float DistanceMeters = FVector::Dist2D(Bike->GetActorLocation(), RivalBike->GetActorLocation()) / 100.0f;
        DrawText(
            FString::Printf(TEXT("MAD: %s [%s]  %.0fm"), *RivalName, *AI->GetPersonalityLabel(), DistanceMeters),
            FLinearColor(1.0f, 0.25f, 0.10f),
            28.0f,
            DamageText.IsEmpty() ? 170.0f : 192.0f,
            Font,
            0.82f,
            false);
        break;
    }

    if (bHasRaceProgress && PlayerProgress.bFinished)
    {
        DrawRect(FLinearColor(0.015f, 0.022f, 0.030f, 0.70f), Canvas->SizeX * 0.34f, Canvas->SizeY * 0.31f, 520.0f, 115.0f);
        DrawText(
            FString::Printf(TEXT("FINISH!   PLACE %d/%d"), PlayerPlace, ParticipantCount),
            FLinearColor(0.20f, 1.0f, 0.32f),
            Canvas->SizeX * 0.39f,
            Canvas->SizeY * 0.34f,
            Font,
            2.1f,
            false);
        DrawText(
            FString::Printf(TEXT("TIME %s     ENTER TO RACE AGAIN"), *RaceTimeText),
            FLinearColor(1.0f, 0.86f, 0.22f),
            Canvas->SizeX * 0.37f,
            Canvas->SizeY * 0.40f,
            Font,
            1.25f,
            false);
    }

    const float ControlsY = Canvas->SizeY - 47.0f;
    DrawRect(FLinearColor(0.015f, 0.022f, 0.030f, 0.52f), 16.0f, ControlsY - 8.0f, 570.0f, 34.0f);
    DrawText(
        TEXT("W/S drive  A/D steer  |  Q/E slap  F peel  G egg  R recover  ENTER restart"),
        FLinearColor(0.76f, 0.86f, 1.0f),
        28.0f,
        ControlsY,
        Font,
        0.78f,
        false);
}
