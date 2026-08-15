#include "Presentation/RITrafficStyleSubsystem.h"

#include "Traffic/RITrafficVehicle.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    const FLinearColor RITSTYLE_GlassColor(0.025f, 0.075f, 0.095f, 1.0f);
    const FLinearColor RITSTYLE_ChromeColor(0.42f, 0.44f, 0.46f, 1.0f);
    const FLinearColor RITSTYLE_TaxiSignColor(1.0f, 0.82f, 0.16f, 1.0f);
    const FLinearColor RITSTYLE_TaxiStripeColor(0.92f, 0.92f, 0.82f, 1.0f);
    const FLinearColor RITSTYLE_VanCargoColor(0.58f, 0.11f, 0.035f, 1.0f);
}

bool URITrafficStyleSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bStyled && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URITrafficStyleSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URITrafficStyleSubsystem, STATGROUP_Tickables);
}

UStaticMeshComponent* URITrafficStyleSubsystem::FindMeshComponent(
    ARITrafficVehicle* Vehicle,
    const FName ComponentName) const
{
    if (!Vehicle)
    {
        return nullptr;
    }

    TArray<UStaticMeshComponent*> Components;
    Vehicle->GetComponents<UStaticMeshComponent>(Components);
    for (UStaticMeshComponent* Component : Components)
    {
        if (Component && Component->GetFName() == ComponentName)
        {
            return Component;
        }
    }
    return nullptr;
}

UStaticMeshComponent* URITrafficStyleSubsystem::CreateDetail(
    ARITrafficVehicle* Vehicle,
    const FName ComponentName,
    const FVector& RelativeLocation,
    const FRotator& RelativeRotation,
    const FVector& RelativeScale,
    const FLinearColor& Color)
{
    if (!Vehicle || !Vehicle->GetRootComponent() || !CubeMesh || !BasicMaterial)
    {
        return nullptr;
    }

    if (UStaticMeshComponent* Existing = FindMeshComponent(Vehicle, ComponentName))
    {
        return Existing;
    }

    UStaticMeshComponent* Detail = NewObject<UStaticMeshComponent>(Vehicle, ComponentName);
    if (!Detail)
    {
        return nullptr;
    }

    Vehicle->AddInstanceComponent(Detail);
    Detail->SetupAttachment(Vehicle->GetRootComponent());
    Detail->SetStaticMesh(CubeMesh);
    Detail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Detail->SetCollisionProfileName(TEXT("NoCollision"));
    Detail->SetGenerateOverlapEvents(false);
    Detail->SetCastShadow(true);
    Detail->SetRelativeLocation(RelativeLocation);
    Detail->SetRelativeRotation(RelativeRotation);
    Detail->SetRelativeScale3D(RelativeScale);
    Detail->RegisterComponent();

    if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BasicMaterial, Detail))
    {
        Material->SetVectorParameterValue(TEXT("Color"), Color);
        Detail->SetMaterial(0, Material);
    }

    return Detail;
}

void URITrafficStyleSubsystem::SetWheelLayout(
    ARITrafficVehicle* Vehicle,
    const float FrontX,
    const float RearX,
    const float HalfWidth,
    const float Height,
    const FVector& WheelScale)
{
    struct FWheelPlacement
    {
        FName Name;
        FVector Location;
    };

    const FWheelPlacement Wheels[] =
    {
        {TEXT("FrontWheelLeft"), FVector(FrontX, -HalfWidth, Height)},
        {TEXT("FrontWheelRight"), FVector(FrontX, HalfWidth, Height)},
        {TEXT("RearWheelLeft"), FVector(RearX, -HalfWidth, Height)},
        {TEXT("RearWheelRight"), FVector(RearX, HalfWidth, Height)}
    };

    for (const FWheelPlacement& Wheel : Wheels)
    {
        if (UStaticMeshComponent* Component = FindMeshComponent(Vehicle, Wheel.Name))
        {
            Component->SetRelativeLocation(Wheel.Location);
            Component->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
            Component->SetRelativeScale3D(WheelScale);
        }
    }
}

void URITrafficStyleSubsystem::StyleVehicle(ARITrafficVehicle* Vehicle, const int32 StyleIndex)
{
    if (!Vehicle)
    {
        return;
    }

    UStaticMeshComponent* Body = FindMeshComponent(Vehicle, TEXT("BodyVisual"));
    UStaticMeshComponent* Cabin = FindMeshComponent(Vehicle, TEXT("CabinVisual"));
    if (!Body || !Cabin)
    {
        return;
    }

    const int32 Style = StyleIndex % 3;
    if (Style == 0)
    {
        // Compact slow sedan: lower roof, short overhangs and simple bumpers.
        Body->SetRelativeLocation(FVector(0.0f, 0.0f, -18.0f));
        Body->SetRelativeScale3D(FVector(3.18f, 1.42f, 0.36f));
        Cabin->SetRelativeLocation(FVector(-18.0f, 0.0f, 27.0f));
        Cabin->SetRelativeScale3D(FVector(1.70f, 1.15f, 0.43f));
        SetWheelLayout(Vehicle, 104.0f, -104.0f, 72.0f, -42.0f, FVector(0.49f, 0.49f, 0.16f));

        CreateDetail(Vehicle, TEXT("VPR22FrontGlass"), FVector(67.0f, 0.0f, 39.0f), FRotator(0.0f, 0.0f, -14.0f), FVector(0.055f, 1.03f, 0.31f), RITSTYLE_GlassColor);
        CreateDetail(Vehicle, TEXT("VPR22RearGlass"), FVector(-96.0f, 0.0f, 36.0f), FRotator(0.0f, 0.0f, 13.0f), FVector(0.05f, 0.98f, 0.28f), RITSTYLE_GlassColor);
        CreateDetail(Vehicle, TEXT("VPR22FrontBumper"), FVector(160.0f, 0.0f, -27.0f), FRotator::ZeroRotator, FVector(0.10f, 1.30f, 0.08f), RITSTYLE_ChromeColor);
        CreateDetail(Vehicle, TEXT("VPR22RearBumper"), FVector(-160.0f, 0.0f, -27.0f), FRotator::ZeroRotator, FVector(0.10f, 1.30f, 0.08f), RITSTYLE_ChromeColor);
    }
    else if (Style == 1)
    {
        // Taxi: sedan silhouette plus unmistakable roof sign and side stripe.
        Body->SetRelativeLocation(FVector(0.0f, 0.0f, -15.0f));
        Body->SetRelativeScale3D(FVector(3.28f, 1.46f, 0.39f));
        Cabin->SetRelativeLocation(FVector(-8.0f, 0.0f, 31.0f));
        Cabin->SetRelativeScale3D(FVector(1.82f, 1.20f, 0.48f));
        SetWheelLayout(Vehicle, 108.0f, -108.0f, 74.0f, -43.0f, FVector(0.52f, 0.52f, 0.17f));

        CreateDetail(Vehicle, TEXT("VPR22TaxiGlass"), FVector(74.0f, 0.0f, 45.0f), FRotator(0.0f, 0.0f, -13.0f), FVector(0.055f, 1.07f, 0.34f), RITSTYLE_GlassColor);
        CreateDetail(Vehicle, TEXT("VPR22TaxiSign"), FVector(-4.0f, 0.0f, 84.0f), FRotator::ZeroRotator, FVector(0.48f, 0.24f, 0.14f), RITSTYLE_TaxiSignColor);
        CreateDetail(Vehicle, TEXT("VPR22TaxiStripeLeft"), FVector(0.0f, -74.0f, -4.0f), FRotator::ZeroRotator, FVector(1.90f, 0.025f, 0.10f), RITSTYLE_TaxiStripeColor);
        CreateDetail(Vehicle, TEXT("VPR22TaxiStripeRight"), FVector(0.0f, 74.0f, -4.0f), FRotator::ZeroRotator, FVector(1.90f, 0.025f, 0.10f), RITSTYLE_TaxiStripeColor);
    }
    else
    {
        // Delivery van: high cargo body, forward cab and rear-door break line.
        Body->SetRelativeLocation(FVector(-32.0f, 0.0f, -9.0f));
        Body->SetRelativeScale3D(FVector(3.35f, 1.56f, 0.48f));
        Cabin->SetRelativeLocation(FVector(104.0f, 0.0f, 31.0f));
        Cabin->SetRelativeScale3D(FVector(1.10f, 1.28f, 0.58f));
        SetWheelLayout(Vehicle, 112.0f, -112.0f, 79.0f, -43.0f, FVector(0.55f, 0.55f, 0.18f));

        CreateDetail(Vehicle, TEXT("VPR22VanCargo"), FVector(-66.0f, 0.0f, 48.0f), FRotator::ZeroRotator, FVector(1.82f, 1.42f, 0.72f), RITSTYLE_VanCargoColor);
        CreateDetail(Vehicle, TEXT("VPR22VanGlass"), FVector(155.0f, 0.0f, 48.0f), FRotator(0.0f, 0.0f, -5.0f), FVector(0.055f, 1.10f, 0.38f), RITSTYLE_GlassColor);
        CreateDetail(Vehicle, TEXT("VPR22VanRearDoorLine"), FVector(-160.0f, 0.0f, 47.0f), FRotator::ZeroRotator, FVector(0.035f, 1.24f, 0.64f), RITSTYLE_ChromeColor);
        CreateDetail(Vehicle, TEXT("VPR22VanBumper"), FVector(-176.0f, 0.0f, -27.0f), FRotator::ZeroRotator, FVector(0.10f, 1.35f, 0.09f), RITSTYLE_ChromeColor);
    }
}

void URITrafficStyleSubsystem::TryStyleTraffic()
{
    UWorld* World = GetWorld();
    if (bStyled || !World)
    {
        return;
    }

    CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    BasicMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!CubeMesh || !BasicMaterial)
    {
        return;
    }

    TArray<ARITrafficVehicle*> Vehicles;
    for (TActorIterator<ARITrafficVehicle> It(World); It; ++It)
    {
        if (*It)
        {
            Vehicles.Add(*It);
        }
    }

    if (Vehicles.Num() == 0)
    {
        // Traffic can now legitimately be set to zero from the race setup menu.
        // Keep ticking briefly until the traffic spawner has had a chance to run.
        return;
    }

    // Styles repeat for counts above three; the mechanical traffic class remains
    // identical and only receives collision-free presentation components.
    for (int32 Index = 0; Index < Vehicles.Num(); ++Index)
    {
        StyleVehicle(Vehicles[Index], Index);
    }

    bStyled = true;
}

void URITrafficStyleSubsystem::Tick(const float DeltaTime)
{
    TryStyleTraffic();
}
