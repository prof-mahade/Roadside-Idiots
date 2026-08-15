#include "Debug/RIRaceSetupHUD.h"

#include "Core/RIPlayerController.h"
#include "Core/RIRaceSettingsSubsystem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameUserSettings.h"

namespace
{
    FString RIQualityLabel(const int32 Quality)
    {
        switch (Quality)
        {
        case 0: return TEXT("LOW");
        case 1: return TEXT("MEDIUM");
        case 2: return TEXT("HIGH");
        case 3: return TEXT("EPIC");
        default: return TEXT("CUSTOM");
        }
    }
}

void ARIRaceSetupHUD::DrawHUD()
{
    ARIPlayerController* RIController = Cast<ARIPlayerController>(PlayerOwner);
    if (!RIController)
    {
        Super::DrawHUD();
        return;
    }

    if (RIController->IsRaceSetupMenuOpen())
    {
        DrawRaceSetupMenu();
        return;
    }

    if (RIController->IsSettingsMenuOpen())
    {
        // When settings was opened from Pause there is a pawn/race behind the
        // panel; from the title/setup screen there is not. Both paths share the
        // same settings UI and persistent UGameUserSettings values.
        if (RIController->GetPawn())
        {
            Super::DrawHUD();
            DrawGameplayMenuHints();
        }
        DrawSettingsMenu();
        return;
    }

    Super::DrawHUD();
    DrawGameplayMenuHints();

    if (RIController->IsPauseMenuOpen())
    {
        DrawPauseMenu();
    }
}

void ARIRaceSetupHUD::DrawGameplayMenuHints()
{
    if (!Canvas || !GEngine) return;

    UFont* Font = GEngine->GetSmallFont();
    if (!Font) return;

    // Cover only the old inherited milestone text rather than reopening the
    // accepted gameplay HUD implementation.
    DrawRect(FLinearColor(0.015f, 0.022f, 0.030f, 0.96f), 24.0f, 45.0f, 310.0f, 19.0f);
    DrawText(
        TEXT("VPR-23B | DEMO FLOW + CHAOS AI"),
        FLinearColor(0.52f, 1.0f, 0.70f),
        28.0f,
        48.0f,
        Font,
        0.76f,
        false);

    // Replace the inherited control strip so the pause key is discoverable.
    const float ControlsY = Canvas->SizeY - 47.0f;
    DrawRect(FLinearColor(0.015f, 0.022f, 0.030f, 0.94f), 16.0f, ControlsY - 8.0f, 700.0f, 34.0f);
    DrawText(
        TEXT("W/S drive  A/D steer | Q/E slap  F peel  G egg  R recover | P pause  ENTER restart"),
        FLinearColor(0.76f, 0.86f, 1.0f),
        28.0f,
        ControlsY,
        Font,
        0.78f,
        false);
}

void ARIRaceSetupHUD::DrawRaceSetupMenu()
{
    if (!Canvas || !PlayerOwner || !GetWorld()) return;

    UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
    if (!Font) return;

    ARIPlayerController* RIController = Cast<ARIPlayerController>(PlayerOwner);
    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    URIRaceSettingsSubsystem* Settings = GameInstance
        ? GameInstance->GetSubsystem<URIRaceSettingsSubsystem>()
        : nullptr;
    if (!RIController || !Settings) return;

    const float ScreenW = Canvas->SizeX;
    const float ScreenH = Canvas->SizeY;
    const float PanelW = FMath::Min(660.0f, ScreenW * 0.74f);
    const float PanelH = FMath::Min(600.0f, ScreenH * 0.82f);
    const float PanelX = (ScreenW - PanelW) * 0.5f;
    const float PanelY = FMath::Max(28.0f, (ScreenH - PanelH) * 0.46f);

    DrawRect(FLinearColor(0.008f, 0.014f, 0.021f, 0.90f), PanelX, PanelY, PanelW, PanelH);
    DrawRect(FLinearColor(0.95f, 0.70f, 0.10f, 0.95f), PanelX, PanelY, PanelW, 6.0f);

    DrawText(
        TEXT("ROADSIDE IDIOTS"),
        FLinearColor(1.0f, 0.77f, 0.16f),
        PanelX + 34.0f,
        PanelY + 28.0f,
        Font,
        2.15f,
        false);

    DrawText(
        TEXT("THE ROAD IS DANGEROUS. THE RIDERS ARE WORSE."),
        FLinearColor(0.70f, 0.80f, 0.88f),
        PanelX + 37.0f,
        PanelY + 72.0f,
        Font,
        0.90f,
        false);

    DrawText(
        TEXT("DEMO 1 - RACE SETUP"),
        FLinearColor(0.52f, 1.0f, 0.70f),
        PanelX + 37.0f,
        PanelY + 102.0f,
        Font,
        0.88f,
        false);

    const FString TrafficValue = Settings->GetTrafficCount() == 0
        ? TEXT("OFF")
        : FString::FromInt(Settings->GetTrafficCount());

    const FString Rows[6] =
    {
        FString::Printf(TEXT("OPPONENTS        <  %d  >     (2 - 6)"), Settings->GetOpponentCount()),
        FString::Printf(TEXT("LAPS             <  %d  >     (1 - 5)"), Settings->GetLapCount()),
        FString::Printf(TEXT("TRAFFIC          <  %s  >     (0 - 6)"), *TrafficValue),
        TEXT("START RACE"),
        TEXT("SETTINGS"),
        TEXT("QUIT GAME")
    };

    constexpr float RowHeight = 56.0f;
    const float RowStartY = PanelY + 154.0f;
    for (int32 Row = 0; Row < 6; ++Row)
    {
        const float Y = RowStartY + static_cast<float>(Row) * RowHeight;
        const bool bSelected = RIController->GetSelectedMenuRow() == Row;

        if (bSelected)
        {
            DrawRect(FLinearColor(0.95f, 0.69f, 0.08f, Row == 3 ? 0.32f : 0.18f), PanelX + 28.0f, Y - 10.0f, PanelW - 56.0f, 43.0f);
        }

        FLinearColor TextColor = bSelected
            ? FLinearColor(1.0f, 0.84f, 0.32f)
            : FLinearColor(0.92f, 0.95f, 0.98f);

        if (Row == 3)
        {
            TextColor = bSelected ? FLinearColor(0.35f, 1.0f, 0.45f) : FLinearColor(0.68f, 0.88f, 0.70f);
        }
        else if (Row == 5)
        {
            TextColor = bSelected ? FLinearColor(1.0f, 0.42f, 0.32f) : FLinearColor(0.88f, 0.55f, 0.50f);
        }

        DrawText(Rows[Row], TextColor, PanelX + 44.0f, Y, Font, Row >= 3 ? 1.18f : 1.08f, false);
    }

    DrawText(
        TEXT("UP/DOWN select    LEFT/RIGHT change    ENTER confirm"),
        FLinearColor(0.70f, 0.75f, 0.80f),
        PanelX + 38.0f,
        PanelY + PanelH - 40.0f,
        Font,
        0.88f,
        false);
}

void ARIRaceSetupHUD::DrawPauseMenu()
{
    if (!Canvas || !PlayerOwner) return;

    UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
    ARIPlayerController* RIController = Cast<ARIPlayerController>(PlayerOwner);
    if (!Font || !RIController) return;

    const float PanelW = FMath::Min(500.0f, Canvas->SizeX * 0.62f);
    const float PanelH = 390.0f;
    const float PanelX = (Canvas->SizeX - PanelW) * 0.5f;
    const float PanelY = (Canvas->SizeY - PanelH) * 0.46f;

    DrawRect(FLinearColor(0.006f, 0.011f, 0.018f, 0.91f), PanelX, PanelY, PanelW, PanelH);
    DrawRect(FLinearColor(0.95f, 0.70f, 0.10f, 0.95f), PanelX, PanelY, PanelW, 5.0f);
    DrawText(TEXT("PAUSED"), FLinearColor(1.0f, 0.78f, 0.18f), PanelX + 34.0f, PanelY + 28.0f, Font, 1.9f, false);

    const FString Rows[5] =
    {
        TEXT("RESUME"),
        TEXT("RESTART RACE"),
        TEXT("CHANGE RACE SETUP"),
        TEXT("SETTINGS"),
        TEXT("QUIT GAME")
    };

    constexpr float RowHeight = 51.0f;
    const float RowStartY = PanelY + 91.0f;
    for (int32 Row = 0; Row < 5; ++Row)
    {
        const float Y = RowStartY + Row * RowHeight;
        const bool bSelected = RIController->GetSelectedMenuRow() == Row;
        if (bSelected)
        {
            DrawRect(FLinearColor(0.95f, 0.69f, 0.08f, 0.22f), PanelX + 26.0f, Y - 9.0f, PanelW - 52.0f, 40.0f);
        }

        FLinearColor Color = bSelected ? FLinearColor(1.0f, 0.85f, 0.34f) : FLinearColor(0.90f, 0.94f, 0.98f);
        if (Row == 0) Color = bSelected ? FLinearColor(0.35f, 1.0f, 0.45f) : FLinearColor(0.67f, 0.86f, 0.70f);
        if (Row == 4) Color = bSelected ? FLinearColor(1.0f, 0.42f, 0.32f) : FLinearColor(0.88f, 0.55f, 0.50f);
        DrawText(Rows[Row], Color, PanelX + 42.0f, Y, Font, 1.12f, false);
    }

    DrawText(
        TEXT("P / ESC resume    UP/DOWN select    ENTER confirm"),
        FLinearColor(0.68f, 0.74f, 0.80f),
        PanelX + 34.0f,
        PanelY + PanelH - 36.0f,
        Font,
        0.82f,
        false);
}

void ARIRaceSetupHUD::DrawSettingsMenu()
{
    if (!Canvas || !PlayerOwner) return;

    UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
    ARIPlayerController* RIController = Cast<ARIPlayerController>(PlayerOwner);
    UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
    if (!Font || !RIController || !UserSettings) return;

    const float PanelW = FMath::Min(540.0f, Canvas->SizeX * 0.66f);
    const float PanelH = 315.0f;
    const float PanelX = (Canvas->SizeX - PanelW) * 0.5f;
    const float PanelY = (Canvas->SizeY - PanelH) * 0.46f;

    DrawRect(FLinearColor(0.006f, 0.011f, 0.018f, 0.93f), PanelX, PanelY, PanelW, PanelH);
    DrawRect(FLinearColor(0.34f, 0.72f, 1.0f, 0.95f), PanelX, PanelY, PanelW, 5.0f);
    DrawText(TEXT("SETTINGS"), FLinearColor(0.55f, 0.82f, 1.0f), PanelX + 34.0f, PanelY + 28.0f, Font, 1.75f, false);

    const int32 Quality = UserSettings->GetOverallScalabilityLevel();
    const FString Rows[3] =
    {
        FString::Printf(TEXT("GRAPHICS QUALITY     <  %s  >"), *RIQualityLabel(Quality)),
        FString::Printf(TEXT("VSYNC                <  %s  >"), UserSettings->IsVSyncEnabled() ? TEXT("ON") : TEXT("OFF")),
        TEXT("BACK")
    };

    constexpr float RowHeight = 56.0f;
    const float RowStartY = PanelY + 91.0f;
    for (int32 Row = 0; Row < 3; ++Row)
    {
        const float Y = RowStartY + Row * RowHeight;
        const bool bSelected = RIController->GetSelectedMenuRow() == Row;
        if (bSelected)
        {
            DrawRect(FLinearColor(0.24f, 0.62f, 0.92f, 0.22f), PanelX + 26.0f, Y - 9.0f, PanelW - 52.0f, 41.0f);
        }

        const FLinearColor Color = bSelected
            ? FLinearColor(0.55f, 0.88f, 1.0f)
            : FLinearColor(0.90f, 0.94f, 0.98f);
        DrawText(Rows[Row], Color, PanelX + 42.0f, Y, Font, Row == 2 ? 1.12f : 1.02f, false);
    }

    DrawText(
        TEXT("LEFT/RIGHT change    ENTER on BACK    P/ESC back"),
        FLinearColor(0.68f, 0.74f, 0.80f),
        PanelX + 34.0f,
        PanelY + PanelH - 34.0f,
        Font,
        0.80f,
        false);
}
