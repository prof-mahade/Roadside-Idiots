#include "Visual/RIDamagePresentationSubsystem.h"

#include "Vehicle/RIBikePawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
    const FName ArmBandageName(TEXT("PrototypeBandageArm"));
    const FName HeadBandageName(TEXT("PrototypeBandageHead"));
    const FName LegBandageName(TEXT("PrototypeBandageLeg"));
    const FName ArmAccentName(TEXT("PrototypeBandageArmAccent"));
    const FName HeadAccentName(TEXT("PrototypeBandageHeadAccent"));
    const FName LegAccentName(TEXT("PrototypeBandageLegAccent"));
}

bool URIDamagePresentationSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIDamagePresentationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIDamagePresentationSubsystem, STATGROUP_Tickables);
}

UStaticMeshComponent* URIDamagePresentationSubsystem::FindStaticComponent(ARIBikePawn* Bike, const FName ComponentName) const
{
    if (!Bike) return nullptr;

    TArray<UStaticMeshComponent*> Components;
    Bike->GetComponents<UStaticMeshComponent>(Components);
    for (UStaticMeshComponent* Component : Components)
    {
        if (Component && Component->GetFName() == ComponentName)
        {
            return Component;
        }
    }
    return nullptr;
}

UStaticMeshComponent* URIDamagePresentationSubsystem::EnsureAccent(
    ARIBikePawn* Bike,
    UStaticMeshComponent* ParentBandage,
    const FName AccentName,
    const FVector RelativeScale)
{
    if (!Bike || !ParentBandage) return nullptr;

    if (UStaticMeshComponent* Existing = FindStaticComponent(Bike, AccentName))
    {
        Existing->SetVisibility(ParentBandage->IsVisible(), true);
        return Existing;
    }

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!CubeMesh) return nullptr;

    UStaticMeshComponent* Accent = NewObject<UStaticMeshComponent>(Bike, AccentName);
    Bike->AddInstanceComponent(Accent);
    Accent->SetupAttachment(ParentBandage);
    Accent->SetStaticMesh(CubeMesh);
    Accent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Accent->SetGenerateOverlapEvents(false);
    Accent->SetRelativeLocation(FVector::ZeroVector);
    Accent->SetRelativeRotation(FRotator::ZeroRotator);
    Accent->SetRelativeScale3D(RelativeScale);
    Accent->RegisterComponent();
    Accent->SetVisibility(ParentBandage->IsVisible(), true);

    if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
    {
        if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Accent))
        {
            Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.60f, 0.025f, 0.02f, 1.0f));
            Accent->SetMaterial(0, Material);
        }
    }

    return Accent;
}

void URIDamagePresentationSubsystem::UpdateBike(ARIBikePawn* Bike)
{
    if (!Bike) return;

    UStaticMeshComponent* Arm = FindStaticComponent(Bike, ArmBandageName);
    UStaticMeshComponent* Head = FindStaticComponent(Bike, HeadBandageName);
    UStaticMeshComponent* Leg = FindStaticComponent(Bike, LegBandageName);

    if (Arm)
    {
        Arm->SetRelativeScale3D(FVector(0.22f, 0.145f, 0.145f));
        EnsureAccent(Bike, Arm, ArmAccentName, FVector(1.04f, 0.20f, 1.04f));
    }

    if (Head)
    {
        Head->SetRelativeLocation(FVector(-2.0f, 0.0f, 8.0f));
        Head->SetRelativeScale3D(FVector(0.24f, 0.35f, 0.085f));
        EnsureAccent(Bike, Head, HeadAccentName, FVector(0.18f, 1.04f, 1.05f));
    }

    if (Leg)
    {
        Leg->SetRelativeScale3D(FVector(0.23f, 0.135f, 0.135f));
        EnsureAccent(Bike, Leg, LegAccentName, FVector(1.04f, 0.22f, 1.04f));
    }
}

void URIDamagePresentationSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        UpdateBike(*It);
    }
}
