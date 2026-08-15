#include "Debug/RIRaceSetupHUD.h"

#include "Core/RIPlayerController.h"
#include "Core/RIRaceSettingsSubsystem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

void ARIRaceSetupHUD::DrawHUD()
{
    ARIPlayerController* RIController = Cast<ARIPlayerController>(PlayerOwner);
    if (RIController && RIController->IsRaceSetupMenuOpen())
    {
        DrawRaceSetupMenu();
        return;
    }

    Super::DrawHUD();

    // The proven gameplay HUD still contains the old VPR-20.1 build string.
    // Cover only that small line here instead of rewriting the large accepted HUD.
    if (Canvas && GEngine)
    {
        if (UFont* Font = GEngine->GetSmallFont())
        {
            DrawRect(FLinearColor(0.015f, 0.022f, 0.030f, 0.96f), 24.0f, 45.0f, 300.0f, 19.0f);
            DrawText(
                TEXT("VPR-23A | CONFIG RACE + CHAOS AI"),
                FLinearColor(0.52f, 1.0f, 0.70f),
                28.0f,
                48.0f,
                Font,
                0.76f,
                false);
        }
    }
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
    const float PanelW = FMath::Min(620.0f, ScreenW * 0.72f);
    const float PanelH = 470.0f;
    const float PanelX = (ScreenW - PanelW) * 0.5f;
    const float PanelY = FMath::Max(42.0f, (ScreenH - PanelH) * 0.46f);

    DrawRect(FLinearColor(0.008f, 0.014f, 0.021f, 0.88f), PanelX, PanelY, PanelW, PanelH);
    DrawRect(FLinearColor(0.95f, 0.70f, 0.10f, 0.92f), PanelX, PanelY, PanelW, 5.0f);

    DrawText(
        TEXT("ROADSIDE IDIOTS"),
        FLinearColor(1.0f, 0.77f, 0.16f),
        PanelX + 34.0f,
        PanelY + 30.0f,
        Font,
        2.0f,
        false);

    DrawText(
        TEXT("RACE SETUP"),
        FLinearColor(0.78f, 0.86f, 0.93f),
        PanelX + 37.0f,
        PanelY + 72.0f,
        Font,
        1.0f,
        false);

    DrawText(
        TEXT("Some rivals race. Some rivals are here to ruin everyone's day."),
        FLinearColor(0.62f, 0.69f, 0.75f),
        PanelX + 37.0f,
        PanelY + 101.0f,
        Font,
        0.88f,
        false);

    const FString TrafficValue = Settings->GetTrafficCount() == 0
        ? TEXT("OFF")
        : FString::FromInt(Settings->GetTrafficCount());

    const FString Rows[4] =
    {
        FString::Printf(TEXT("OPPONENTS        <  %d  >     (2 - 6)"), Settings->GetOpponentCount()),
        FString::Printf(TEXT("LAPS             <  %d  >     (1 - 5)"), Settings->GetLapCount()),
        FString::Printf(TEXT("TRAFFIC          <  %s  >     (0 - 6)"), *TrafficValue),
        TEXT("START RACE")
    };

    constexpr float RowHeight = 58.0f;
    const float RowStartY = PanelY + 150.0f;
    for (int32 Row = 0; Row < 4; ++Row)
    {
        const float Y = RowStartY + static_cast<float>(Row) * RowHeight;
        const bool bSelected = RIController->GetSelectedSetupRow() == Row;

        if (bSelected)
        {
            DrawRect(FLinearColor(0.95f, 0.69f, 0.08f, Row == 3 ? 0.32f : 0.18f), PanelX + 28.0f, Y - 10.0f, PanelW - 56.0f, 44.0f);
        }

        const FLinearColor TextColor = Row == 3
            ? (bSelected ? FLinearColor(0.35f, 1.0f, 0.45f) : FLinearColor(0.68f, 0.82f, 0.70f))
            : (bSelected ? FLinearColor(1.0f, 0.84f, 0.32f) : FLinearColor(0.92f, 0.95f, 0.98f));

        DrawText(Rows[Row], TextColor, PanelX + 44.0f, Y, Font, Row == 3 ? 1.25f : 1.10f, false);
    }

    DrawText(
        TEXT("UP/DOWN select    LEFT/RIGHT change    ENTER start"),
        FLinearColor(0.70f, 0.75f, 0.80f),
        PanelX + 38.0f,
        PanelY + PanelH - 42.0f,
        Font,
        0.88f,
        false);
}
