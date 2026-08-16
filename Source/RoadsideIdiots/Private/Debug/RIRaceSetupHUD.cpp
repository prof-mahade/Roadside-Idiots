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

    FString RIChaosLabel(const int32 ChaosLevel)
    {
        switch (ChaosLevel)
        {
        case 0: return TEXT("CLEAN");
        case 2: return TEXT("MAYHEM");
        default: return TEXT("BALANCED");
        }
    }

    FString RIChaosDescription(const int32 ChaosLevel)
    {
        switch (ChaosLevel)
        {
        case 0: return TEXT("CLEAN: rivals mostly race; deliberate trouble is uncommon.");
        case 2: return TEXT("MAYHEM: more frequent petty incidents; driving skill stays intact.");
        default: return TEXT("BALANCED: the intended mix of racing, traffic and petty chaos.");
        }
    }

    FString RISteeringFeelLabel(const int32 SteeringFeel)
    {
        switch (SteeringFeel)
        {
        case 0: return TEXT("CALM");
        case 2: return TEXT("QUICK");
        default: return TEXT("NORMAL");
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
        if (RIController->GetPawn())
        {
            Super::DrawHUD();
        }
        DrawSettingsMenu();
        return;
    }

    Super::DrawHUD();

    if (RIController->IsPauseMenuOpen())
    {
        DrawPauseMenu();
    }
    else if (RIController->IsFinishScreenActive() && Canvas)
    {
        UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
        if (Font)
        {
            DrawText(
                TEXT("ESC / B  MAIN MENU"),
                FLinearColor(0.70f, 0.84f, 1.0f),
                Canvas->SizeX * 0.415f,
                Canvas->SizeY * 0.458f,
                Font,
                0.94f,
                false);
        }
    }
}

void ARIRaceSetupHUD::DrawGameplayMenuHints()
{
    // Player-facing gameplay information now lives in RIDebugHUD. This wrapper
    // intentionally draws nothing so Shipping builds do not stack duplicate
    // controls or internal VPR labels over the race HUD.
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
    const float PanelW = FMath::Min(680.0f, ScreenW * 0.76f);
    const float PanelH = FMath::Min(620.0f, ScreenH * 0.86f);
    const float PanelX = (ScreenW - PanelW) * 0.5f;
    const float PanelY = FMath::Max(24.0f, (ScreenH - PanelH) * 0.46f);

    DrawRect(FLinearColor(0.008f, 0.014f, 0.021f, 0.92f), PanelX, PanelY, PanelW, PanelH);
    DrawRect(FLinearColor(0.95f, 0.70f, 0.10f, 0.95f), PanelX, PanelY, PanelW, 6.0f);

    DrawText(TEXT("ROADSIDE IDIOTS"), FLinearColor(1.0f, 0.77f, 0.16f), PanelX + 34.0f, PanelY + 27.0f, Font, 2.15f, false);
    DrawText(TEXT("THE ROAD IS DANGEROUS. THE RIDERS ARE WORSE."), FLinearColor(0.70f, 0.80f, 0.88f), PanelX + 37.0f, PanelY + 71.0f, Font, 0.90f, false);
    DrawText(TEXT("QUICK RACE"), FLinearColor(0.52f, 1.0f, 0.70f), PanelX + 37.0f, PanelY + 101.0f, Font, 0.92f, false);

    const FString TrafficValue = Settings->GetTrafficCount() == 0 ? TEXT("OFF") : FString::FromInt(Settings->GetTrafficCount());
    const FString Rows[7] =
    {
        FString::Printf(TEXT("OPPONENTS        <  %d  >     (2 - 6)"), Settings->GetOpponentCount()),
        FString::Printf(TEXT("LAPS             <  %d  >     (1 - 5)"), Settings->GetLapCount()),
        FString::Printf(TEXT("TRAFFIC          <  %s  >     (0 - 6)"), *TrafficValue),
        FString::Printf(TEXT("RACE CHAOS       <  %s  >"), *RIChaosLabel(Settings->GetChaosLevel())),
        TEXT("START RACE"),
        TEXT("SETTINGS"),
        TEXT("QUIT GAME")
    };

    constexpr float RowHeight = 50.0f;
    const float RowStartY = PanelY + 145.0f;
    for (int32 Row = 0; Row < 7; ++Row)
    {
        const float Y = RowStartY + static_cast<float>(Row) * RowHeight;
        const bool bSelected = RIController->GetSelectedMenuRow() == Row;
        if (bSelected)
        {
            DrawRect(FLinearColor(0.95f, 0.69f, 0.08f, Row == 4 ? 0.32f : 0.18f), PanelX + 28.0f, Y - 9.0f, PanelW - 56.0f, 39.0f);
        }

        FLinearColor TextColor = bSelected ? FLinearColor(1.0f, 0.84f, 0.32f) : FLinearColor(0.92f, 0.95f, 0.98f);
        if (Row == 4) TextColor = bSelected ? FLinearColor(0.35f, 1.0f, 0.45f) : FLinearColor(0.68f, 0.88f, 0.70f);
        else if (Row == 6) TextColor = bSelected ? FLinearColor(1.0f, 0.42f, 0.32f) : FLinearColor(0.88f, 0.55f, 0.50f);

        DrawText(Rows[Row], TextColor, PanelX + 44.0f, Y, Font, Row >= 4 ? 1.12f : 1.03f, false);
    }

    DrawText(
        RIChaosDescription(Settings->GetChaosLevel()),
        FLinearColor(0.66f, 0.80f, 0.88f),
        PanelX + 38.0f,
        PanelY + PanelH - 64.0f,
        Font,
        0.78f,
        false);

    DrawText(TEXT("ARROWS / D-PAD select & change    ENTER / A confirm"), FLinearColor(0.70f, 0.75f, 0.80f), PanelX + 38.0f, PanelY + PanelH - 37.0f, Font, 0.84f, false);
}

void ARIRaceSetupHUD::DrawPauseMenu()
{
    if (!Canvas || !PlayerOwner) return;

    UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
    ARIPlayerController* RIController = Cast<ARIPlayerController>(PlayerOwner);
    if (!Font || !RIController) return;

    const float PanelW = FMath::Min(520.0f, Canvas->SizeX * 0.64f);
    const float PanelH = 420.0f;
    const float PanelX = (Canvas->SizeX - PanelW) * 0.5f;
    const float PanelY = (Canvas->SizeY - PanelH) * 0.46f;

    DrawRect(FLinearColor(0.006f, 0.011f, 0.018f, 0.91f), PanelX, PanelY, PanelW, PanelH);
    DrawRect(FLinearColor(0.95f, 0.70f, 0.10f, 0.95f), PanelX, PanelY, PanelW, 5.0f);
    DrawText(TEXT("PAUSED"), FLinearColor(1.0f, 0.78f, 0.18f), PanelX + 34.0f, PanelY + 28.0f, Font, 1.9f, false);

    const FString Rows[5] = {TEXT("RESUME"), TEXT("RESTART RACE"), TEXT("MAIN MENU"), TEXT("SETTINGS"), TEXT("QUIT GAME")};
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

    DrawText(TEXT("W/S drive  A/D steer | Q/E slap  F peel  G egg  R recover"), FLinearColor(0.60f, 0.72f, 0.82f), PanelX + 34.0f, PanelY + PanelH - 60.0f, Font, 0.70f, false);
    DrawText(TEXT("P / ESC / MENU resume    D-PAD / ARROWS select    A / ENTER confirm    B back"), FLinearColor(0.68f, 0.74f, 0.80f), PanelX + 34.0f, PanelY + PanelH - 35.0f, Font, 0.70f, false);
}

void ARIRaceSetupHUD::DrawSettingsMenu()
{
    if (!Canvas || !PlayerOwner || !GetWorld()) return;

    UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
    ARIPlayerController* RIController = Cast<ARIPlayerController>(PlayerOwner);
    UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    URIRaceSettingsSubsystem* RaceSettings = GameInstance
        ? GameInstance->GetSubsystem<URIRaceSettingsSubsystem>()
        : nullptr;
    if (!Font || !RIController || !UserSettings || !RaceSettings) return;

    const float PanelW = FMath::Min(560.0f, Canvas->SizeX * 0.68f);
    const float PanelH = 365.0f;
    const float PanelX = (Canvas->SizeX - PanelW) * 0.5f;
    const float PanelY = (Canvas->SizeY - PanelH) * 0.46f;

    DrawRect(FLinearColor(0.006f, 0.011f, 0.018f, 0.93f), PanelX, PanelY, PanelW, PanelH);
    DrawRect(FLinearColor(0.34f, 0.72f, 1.0f, 0.95f), PanelX, PanelY, PanelW, 5.0f);
    DrawText(TEXT("SETTINGS"), FLinearColor(0.55f, 0.82f, 1.0f), PanelX + 34.0f, PanelY + 28.0f, Font, 1.75f, false);

    const int32 Quality = UserSettings->GetOverallScalabilityLevel();
    const FString Rows[4] =
    {
        FString::Printf(TEXT("GRAPHICS QUALITY     <  %s  >"), *RIQualityLabel(Quality)),
        FString::Printf(TEXT("VSYNC                <  %s  >"), UserSettings->IsVSyncEnabled() ? TEXT("ON") : TEXT("OFF")),
        FString::Printf(TEXT("STEERING FEEL        <  %s  >"), *RISteeringFeelLabel(RaceSettings->GetSteeringFeel())),
        TEXT("BACK")
    };

    constexpr float RowHeight = 54.0f;
    const float RowStartY = PanelY + 90.0f;
    for (int32 Row = 0; Row < 4; ++Row)
    {
        const float Y = RowStartY + Row * RowHeight;
        const bool bSelected = RIController->GetSelectedMenuRow() == Row;
        if (bSelected)
        {
            DrawRect(FLinearColor(0.24f, 0.62f, 0.92f, 0.22f), PanelX + 26.0f, Y - 9.0f, PanelW - 52.0f, 41.0f);
        }

        const FLinearColor Color = bSelected ? FLinearColor(0.55f, 0.88f, 1.0f) : FLinearColor(0.90f, 0.94f, 0.98f);
        DrawText(Rows[Row], Color, PanelX + 42.0f, Y, Font, Row == 3 ? 1.12f : 1.00f, false);
    }

    DrawText(TEXT("CALM = finer stick control     NORMAL = linear     QUICK = earlier response"), FLinearColor(0.57f, 0.70f, 0.79f), PanelX + 34.0f, PanelY + PanelH - 58.0f, Font, 0.68f, false);
    DrawText(TEXT("LEFT/RIGHT or D-PAD change    ENTER / A on BACK    P/ESC/MENU back"), FLinearColor(0.68f, 0.74f, 0.80f), PanelX + 34.0f, PanelY + PanelH - 33.0f, Font, 0.72f, false);
}
