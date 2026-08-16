#include "Presentation/RIPresentationGradeSubsystem.h"

#include "Vehicle/RIBikePawn.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/Scene.h"
#include "EngineUtils.h"

bool URIPresentationGradeSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bBuilt && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIPresentationGradeSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIPresentationGradeSubsystem, STATGROUP_Tickables);
}

void URIPresentationGradeSubsystem::Tick(const float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || bBuilt) return;

    bool bRaceWorldExists = false;
    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        if (*It)
        {
            bRaceWorldExists = true;
            break;
        }
    }
    if (!bRaceWorldExists) return;

    BuildGrade();
    bBuilt = true;
}

void URIPresentationGradeSubsystem::BuildGrade()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APostProcessVolume* Volume = World->SpawnActor<APostProcessVolume>();
    if (!Volume) return;

    Volume->SetActorEnableCollision(false);
    Volume->bEnabled = true;
    Volume->bUnbound = true;
    Volume->BlendWeight = 1.0f;
    Volume->Priority = 0.25f;

    FPostProcessSettings& Settings = Volume->Settings;
    Settings.bOverride_ColorContrast = true;
    Settings.ColorContrast = FVector4(1.05f, 1.05f, 1.05f, 1.0f);

    Settings.bOverride_ColorSaturation = true;
    Settings.ColorSaturation = FVector4(1.03f, 1.03f, 1.03f, 1.0f);

    Settings.bOverride_VignetteIntensity = true;
    Settings.VignetteIntensity = 0.08f;

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI PRESENTATION GRADE contrast=1.05 saturation=1.03 vignette=0.08 gameplay=unchanged"));
}