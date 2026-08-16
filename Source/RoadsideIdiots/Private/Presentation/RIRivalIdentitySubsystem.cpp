#include "Presentation/RIRivalIdentitySubsystem.h"

#include "AI/RIAIController.h"
#include "Vehicle/RIBikePawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
    const FName RIPersonalityAccentLeftName(TEXT("RIPersonalityAccentLeft"));
    const FName RIPersonalityAccentRightName(TEXT("RIPersonalityAccentRight"));
    const FName RIPersonalityAccentRearName(TEXT("RIPersonalityAccentRear"));

    UStaticMeshComponent* RIFindStaticComponentByName(ARIBikePawn* Bike, const FName Name)
    {
        if (!Bike) return nullptr;

        TArray<UStaticMeshComponent*> Components;
        Bike->GetComponents<UStaticMeshComponent>(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name)
            {
                return Component;
            }
        }
        return nullptr;
    }

    UStaticMeshComponent* RIEnsureAccent(
        ARIBikePawn* Bike,
        UStaticMesh* CubeMesh,
        UMaterialInterface* BaseMaterial,
        const FName ComponentName,
        const FVector& RelativeLocation,
        const FVector& RelativeScale,
        const FLinearColor& Color)
    {
        if (!Bike || !Bike->GetRootComponent() || !CubeMesh) return nullptr;

        if (UStaticMeshComponent* Existing = RIFindStaticComponentByName(Bike, ComponentName))
        {
            return Existing;
        }

        UStaticMeshComponent* Accent = NewObject<UStaticMeshComponent>(Bike, ComponentName);
        if (!Accent) return nullptr;

        Bike->AddInstanceComponent(Accent);
        Accent->SetupAttachment(Bike->GetRootComponent());
        Accent->SetMobility(EComponentMobility::Movable);
        Accent->SetStaticMesh(CubeMesh);
        Accent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Accent->SetCollisionProfileName(TEXT("NoCollision"));
        Accent->SetGenerateOverlapEvents(false);
        Accent->SetCanEverAffectNavigation(false);
        Accent->SetRelativeLocation(RelativeLocation);
        Accent->SetRelativeScale3D(RelativeScale);
        Accent->RegisterComponent();

        if (BaseMaterial)
        {
            if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Accent))
            {
                Material->SetVectorParameterValue(TEXT("Color"), Color);
                Accent->SetMaterial(0, Material);
            }
        }

        return Accent;
    }
}

bool URIRivalIdentitySubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIRivalIdentitySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIRivalIdentitySubsystem, STATGROUP_Tickables);
}

void URIRivalIdentitySubsystem::Tick(const float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    ScanAccumulator += FMath::Max(0.0f, DeltaTime);
    if (ScanAccumulator < 0.65f) return;
    ScanAccumulator = 0.0f;

    if (!CubeMesh)
    {
        CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    }
    if (!BaseMaterial)
    {
        BaseMaterial = LoadObject<UMaterialInterface>(
            nullptr,
            TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    }
    if (!CubeMesh) return;

    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        ARIBikePawn* Bike = *It;
        ARIAIController* AI = Bike ? Cast<ARIAIController>(Bike->GetController()) : nullptr;
        if (!Bike || !AI) continue;

        EnsureIdentity(Bike, AI->GetPersonalityLabel());
    }
}

FLinearColor URIRivalIdentitySubsystem::ColorForPersonality(const FString& PersonalityLabel) const
{
    if (PersonalityLabel.Equals(TEXT("LEECH"), ESearchCase::IgnoreCase))
    {
        return FLinearColor(0.10f, 0.82f, 0.54f, 1.0f);
    }
    if (PersonalityLabel.Equals(TEXT("HOTHEAD"), ESearchCase::IgnoreCase))
    {
        return FLinearColor(1.0f, 0.30f, 0.06f, 1.0f);
    }
    if (PersonalityLabel.Equals(TEXT("PETTY"), ESearchCase::IgnoreCase))
    {
        return FLinearColor(0.72f, 0.28f, 0.96f, 1.0f);
    }
    if (PersonalityLabel.Equals(TEXT("GREMLIN"), ESearchCase::IgnoreCase))
    {
        return FLinearColor(0.58f, 0.88f, 0.08f, 1.0f);
    }
    if (PersonalityLabel.Equals(TEXT("BRAWLER"), ESearchCase::IgnoreCase))
    {
        return FLinearColor(0.88f, 0.08f, 0.12f, 1.0f);
    }
    if (PersonalityLabel.Equals(TEXT("TRYHARD"), ESearchCase::IgnoreCase))
    {
        return FLinearColor(0.12f, 0.52f, 1.0f, 1.0f);
    }
    return FLinearColor(0.80f, 0.82f, 0.86f, 1.0f);
}

void URIRivalIdentitySubsystem::EnsureIdentity(ARIBikePawn* Bike, const FString& PersonalityLabel)
{
    if (!Bike || !Bike->GetRootComponent() || !CubeMesh) return;

    const FLinearColor PersonalityColor = ColorForPersonality(PersonalityLabel);

    // Thin body accents keep personality recognition close to the motorcycle.
    // The original tall flag/pole was readable but visually dominated the rider
    // and could look like it was dangling below the bike from the chase camera.
    RIEnsureAccent(
        Bike,
        CubeMesh,
        BaseMaterial,
        RIPersonalityAccentLeftName,
        FVector(-34.0f, -43.0f, 48.0f),
        FVector(0.52f, 0.055f, 0.095f),
        PersonalityColor);

    RIEnsureAccent(
        Bike,
        CubeMesh,
        BaseMaterial,
        RIPersonalityAccentRightName,
        FVector(-34.0f, 43.0f, 48.0f),
        FVector(0.52f, 0.055f, 0.095f),
        PersonalityColor);

    const bool bHadRearAccent = RIFindStaticComponentByName(Bike, RIPersonalityAccentRearName) != nullptr;
    RIEnsureAccent(
        Bike,
        CubeMesh,
        BaseMaterial,
        RIPersonalityAccentRearName,
        FVector(-91.0f, 0.0f, 57.0f),
        FVector(0.13f, 0.27f, 0.095f),
        PersonalityColor);

    if (!bHadRearAccent)
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("RI RIVAL IDENTITY personality=%s style=body_accents collision=off"),
            *PersonalityLabel);
    }
}
