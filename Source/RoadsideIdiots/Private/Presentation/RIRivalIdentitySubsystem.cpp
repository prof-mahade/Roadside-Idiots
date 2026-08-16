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
    const FName RIPersonalityPoleName(TEXT("RIPersonalityPole"));
    const FName RIPersonalityFlagName(TEXT("RIPersonalityFlag"));

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

    UStaticMeshComponent* Pole = RIFindStaticComponentByName(Bike, RIPersonalityPoleName);
    if (!Pole)
    {
        Pole = NewObject<UStaticMeshComponent>(Bike, RIPersonalityPoleName);
        if (Pole)
        {
            Bike->AddInstanceComponent(Pole);
            Pole->SetupAttachment(Bike->GetRootComponent());
            Pole->SetMobility(EComponentMobility::Movable);
            Pole->SetStaticMesh(CubeMesh);
            Pole->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Pole->SetCollisionProfileName(TEXT("NoCollision"));
            Pole->SetGenerateOverlapEvents(false);
            Pole->SetCanEverAffectNavigation(false);
            Pole->SetRelativeLocation(FVector(-92.0f, 0.0f, 155.0f));
            Pole->SetRelativeScale3D(FVector(0.035f, 0.035f, 1.65f));
            Pole->RegisterComponent();

            if (BaseMaterial)
            {
                if (UMaterialInstanceDynamic* PoleMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Pole))
                {
                    PoleMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.075f, 0.080f, 0.085f, 1.0f));
                    Pole->SetMaterial(0, PoleMaterial);
                }
            }
        }
    }

    UStaticMeshComponent* Flag = RIFindStaticComponentByName(Bike, RIPersonalityFlagName);
    if (!Flag)
    {
        Flag = NewObject<UStaticMeshComponent>(Bike, RIPersonalityFlagName);
        if (Flag)
        {
            Bike->AddInstanceComponent(Flag);
            Flag->SetupAttachment(Bike->GetRootComponent());
            Flag->SetMobility(EComponentMobility::Movable);
            Flag->SetStaticMesh(CubeMesh);
            Flag->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Flag->SetCollisionProfileName(TEXT("NoCollision"));
            Flag->SetGenerateOverlapEvents(false);
            Flag->SetCanEverAffectNavigation(false);
            Flag->SetRelativeLocation(FVector(-91.0f, 0.0f, 229.0f));
            Flag->SetRelativeScale3D(FVector(0.055f, 0.52f, 0.28f));
            Flag->RegisterComponent();

            if (BaseMaterial)
            {
                if (UMaterialInstanceDynamic* FlagMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Flag))
                {
                    FlagMaterial->SetVectorParameterValue(TEXT("Color"), ColorForPersonality(PersonalityLabel));
                    Flag->SetMaterial(0, FlagMaterial);
                }
            }

            UE_LOG(
                LogTemp,
                Display,
                TEXT("RI RIVAL IDENTITY personality=%s collision=off"),
                *PersonalityLabel);
        }
    }
}
