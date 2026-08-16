#include "Presentation/RITrafficStyleSubsystem.h"

#include "Traffic/RITrafficVehicle.h"
#include "Core/RIRaceSettingsSubsystem.h"
#include "Race/RIRaceManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"
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
    const FLinearColor RITSTYLE_CngCanopyColor(0.055f, 0.16f, 0.075f, 1.0f);
    const FLinearColor RITSTYLE_MicrobusTrimColor(0.78f, 0.78f, 0.72f, 1.0f);
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
            Component->SetVisibility(true, true);
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

    const FString& Label = Vehicle->GetTrafficLabel();

    if (Label.Equals(TEXT("CNG AUTO"), ESearchCase::IgnoreCase))
    {
        // Recognizable three-wheeler proportions without importing a new asset.
        // The mechanical traffic actor already uses a matching compact hit volume.
        Body->SetRelativeLocation(FVector(-4.0f, 0.0f, -8.0f));
        Body->SetRelativeScale3D(FVector(1.72f, 0.96f, 0.50f));
        Cabin->SetRelativeLocation(FVector(-20.0f, 0.0f, 42.0f));
        Cabin->SetRelativeScale3D(FVector(1.20f, 0.88f, 0.82f));

        if (UStaticMeshComponent* FrontWheel = FindMeshComponent(Vehicle, TEXT("FrontWheelLeft")))
        {
            FrontWheel->SetVisibility(true, true);
            FrontWheel->SetRelativeLocation(FVector(82.0f, 0.0f, -38.0f));
            FrontWheel->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
            FrontWheel->SetRelativeScale3D(FVector(0.42f, 0.42f, 0.16f));
        }
        if (UStaticMeshComponent* HiddenFrontWheel = FindMeshComponent(Vehicle, TEXT("FrontWheelRight")))
        {
            HiddenFrontWheel->SetVisibility(false, true);
        }
        if (UStaticMeshComponent* RearLeft = FindMeshComponent(Vehicle, TEXT("RearWheelLeft")))
        {
            RearLeft->SetVisibility(true, true);
            RearLeft->SetRelativeLocation(FVector(-64.0f, -54.0f, -38.0f));
            RearLeft->SetRelativeScale3D(FVector(0.44f, 0.44f, 0.16f));
        }
        if (UStaticMeshComponent* RearRight = FindMeshComponent(Vehicle, TEXT("RearWheelRight")))
        {
            RearRight->SetVisibility(true, true);
            RearRight->SetRelativeLocation(FVector(-64.0f, 54.0f, -38.0f));
            RearRight->SetRelativeScale3D(FVector(0.44f, 0.44f, 0.16f));
        }

        CreateDetail(Vehicle, TEXT("CngFrontGlass"), FVector(50.0f, 0.0f, 61.0f), FRotator(0.0f, 0.0f, -10.0f), FVector(0.045f, 0.71f, 0.43f), RITSTYLE_GlassColor);
        CreateDetail(Vehicle, TEXT("CngCanopy"), FVector(-24.0f, 0.0f, 91.0f), FRotator::ZeroRotator, FVector(1.12f, 0.91f, 0.10f), RITSTYLE_CngCanopyColor);
        CreateDetail(Vehicle, TEXT("CngRearRail"), FVector(-82.0f, 0.0f, 34.0f), FRotator::ZeroRotator, FVector(0.05f, 0.72f, 0.48f), RITSTYLE_ChromeColor);
        return;
    }

    if (Label.Equals(TEXT("TAXI"), ESearchCase::IgnoreCase))
    {
        Body->SetRelativeLocation(FVector(0.0f, 0.0f, -15.0f));
        Body->SetRelativeScale3D(FVector(3.28f, 1.46f, 0.39f));
        Cabin->SetRelativeLocation(FVector(-8.0f, 0.0f, 31.0f));
        Cabin->SetRelativeScale3D(FVector(1.82f, 1.20f, 0.48f));
        SetWheelLayout(Vehicle, 108.0f, -108.0f, 74.0f, -43.0f, FVector(0.52f, 0.52f, 0.17f));

        CreateDetail(Vehicle, TEXT("TaxiGlass"), FVector(74.0f, 0.0f, 45.0f), FRotator(0.0f, 0.0f, -13.0f), FVector(0.055f, 1.07f, 0.34f), RITSTYLE_GlassColor);
        CreateDetail(Vehicle, TEXT("TaxiSign"), FVector(-4.0f, 0.0f, 84.0f), FRotator::ZeroRotator, FVector(0.48f, 0.24f, 0.14f), RITSTYLE_TaxiSignColor);
        CreateDetail(Vehicle, TEXT("TaxiStripeLeft"), FVector(0.0f, -74.0f, -4.0f), FRotator::ZeroRotator, FVector(1.90f, 0.025f, 0.10f), RITSTYLE_TaxiStripeColor);
        CreateDetail(Vehicle, TEXT("TaxiStripeRight"), FVector(0.0f, 74.0f, -4.0f), FRotator::ZeroRotator, FVector(1.90f, 0.025f, 0.10f), RITSTYLE_TaxiStripeColor);
        return;
    }

    if (Label.Equals(TEXT("DELIVERY VAN"), ESearchCase::IgnoreCase))
    {
        Body->SetRelativeLocation(FVector(-28.0f, 0.0f, -7.0f));
        Body->SetRelativeScale3D(FVector(3.45f, 1.54f, 0.54f));
        Cabin->SetRelativeLocation(FVector(105.0f, 0.0f, 35.0f));
        Cabin->SetRelativeScale3D(FVector(1.12f, 1.28f, 0.60f));
        SetWheelLayout(Vehicle, 114.0f, -116.0f, 79.0f, -43.0f, FVector(0.55f, 0.55f, 0.18f));

        CreateDetail(Vehicle, TEXT("VanCargo"), FVector(-68.0f, 0.0f, 54.0f), FRotator::ZeroRotator, FVector(1.86f, 1.43f, 0.76f), RITSTYLE_VanCargoColor);
        CreateDetail(Vehicle, TEXT("VanGlass"), FVector(156.0f, 0.0f, 52.0f), FRotator(0.0f, 0.0f, -5.0f), FVector(0.055f, 1.10f, 0.38f), RITSTYLE_GlassColor);
        CreateDetail(Vehicle, TEXT("VanRearDoorLine"), FVector(-163.0f, 0.0f, 50.0f), FRotator::ZeroRotator, FVector(0.035f, 1.24f, 0.64f), RITSTYLE_ChromeColor);
        return;
    }

    if (Label.Equals(TEXT("MICROBUS"), ESearchCase::IgnoreCase))
    {
        Body->SetRelativeLocation(FVector(-2.0f, 0.0f, -4.0f));
        Body->SetRelativeScale3D(FVector(3.55f, 1.56f, 0.60f));
        Cabin->SetRelativeLocation(FVector(-18.0f, 0.0f, 50.0f));
        Cabin->SetRelativeScale3D(FVector(2.46f, 1.34f, 0.70f));
        SetWheelLayout(Vehicle, 116.0f, -116.0f, 80.0f, -43.0f, FVector(0.56f, 0.56f, 0.18f));

        CreateDetail(Vehicle, TEXT("MicrobusFrontGlass"), FVector(126.0f, 0.0f, 66.0f), FRotator(0.0f, 0.0f, -7.0f), FVector(0.05f, 1.16f, 0.46f), RITSTYLE_GlassColor);
        CreateDetail(Vehicle, TEXT("MicrobusSideTrimLeft"), FVector(-8.0f, -79.0f, 14.0f), FRotator::ZeroRotator, FVector(2.45f, 0.025f, 0.07f), RITSTYLE_MicrobusTrimColor);
        CreateDetail(Vehicle, TEXT("MicrobusSideTrimRight"), FVector(-8.0f, 79.0f, 14.0f), FRotator::ZeroRotator, FVector(2.45f, 0.025f, 0.07f), RITSTYLE_MicrobusTrimColor);
        return;
    }

    // Compact civilian cars. Keep small visual differences by semantic label,
    // but no player-facing behavior depends on these presentation-only details.
    const bool bLostDriver = Label.Equals(TEXT("LOST DRIVER"), ESearchCase::IgnoreCase);
    const float BodyLength = bLostDriver ? 2.88f : 3.02f;
    Body->SetRelativeLocation(FVector(0.0f, 0.0f, -18.0f));
    Body->SetRelativeScale3D(FVector(BodyLength, 1.38f, 0.36f));
    Cabin->SetRelativeLocation(FVector(bLostDriver ? -34.0f : -16.0f, 0.0f, 28.0f));
    Cabin->SetRelativeScale3D(FVector(bLostDriver ? 1.42f : 1.60f, 1.13f, 0.44f));
    SetWheelLayout(Vehicle, 98.0f, -98.0f, 70.0f, -41.0f, FVector(0.48f, 0.48f, 0.16f));

    CreateDetail(Vehicle, TEXT("CompactFrontGlass"), FVector(62.0f, 0.0f, 40.0f), FRotator(0.0f, 0.0f, -13.0f), FVector(0.05f, 0.99f, 0.30f), RITSTYLE_GlassColor);
    CreateDetail(Vehicle, TEXT("CompactFrontBumper"), FVector(148.0f, 0.0f, -27.0f), FRotator::ZeroRotator, FVector(0.09f, 1.23f, 0.07f), RITSTYLE_ChromeColor);
}

void URITrafficStyleSubsystem::TryStyleTraffic()
{
    UWorld* World = GetWorld();
    if (bStyled || !World)
    {
        return;
    }

    // Do not repeatedly load assets and scan the world while the setup menu is
    // still open. ARIRaceManager appears only after START RACE is confirmed.
    bool bRaceReady = false;
    for (TActorIterator<ARIRaceManager> It(World); It; ++It)
    {
        if (*It)
        {
            bRaceReady = true;
            break;
        }
    }
    if (!bRaceReady) return;

    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        if (const URIRaceSettingsSubsystem* Settings = GameInstance->GetSubsystem<URIRaceSettingsSubsystem>())
        {
            if (Settings->GetTrafficCount() <= 0)
            {
                bStyled = true;
                return;
            }
        }
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
        // Traffic spawns in its own world-subsystem tick; wait one more frame.
        return;
    }

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
