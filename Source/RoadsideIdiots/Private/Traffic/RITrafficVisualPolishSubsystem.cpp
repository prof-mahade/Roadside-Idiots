#include "Traffic/RITrafficVisualPolishSubsystem.h"

#include "Traffic/RITrafficVehicle.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

bool URITrafficVisualPolishSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld();
}

TStatId URITrafficVisualPolishSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URITrafficVisualPolishSubsystem, STATGROUP_Tickables);
}

void URITrafficVisualPolishSubsystem::Tick(const float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    ScanAccumulator += FMath::Max(0.0f, DeltaTime);
    if (ScanAccumulator < 0.75f) return;
    ScanAccumulator = 0.0f;

    if (!CubeMesh)
    {
        CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    }
    if (!SphereMesh)
    {
        SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    }
    if (!BaseMaterial)
    {
        BaseMaterial = LoadObject<UMaterialInterface>(
            nullptr,
            TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    }
    if (!CubeMesh) return;

    for (auto It = PolishedTraffic.CreateIterator(); It; ++It)
    {
        if (!It->IsValid())
        {
            It.RemoveCurrent();
        }
    }

    for (TActorIterator<ARITrafficVehicle> It(World); It; ++It)
    {
        ARITrafficVehicle* Traffic = *It;
        if (!Traffic) continue;

        const TWeakObjectPtr<ARITrafficVehicle> TrafficKey(Traffic);
        if (PolishedTraffic.Contains(TrafficKey)) continue;

        EnsurePolish(Traffic);
        PolishedTraffic.Add(TrafficKey);
    }
}

void URITrafficVisualPolishSubsystem::EnsurePolish(ARITrafficVehicle* Traffic)
{
    if (!Traffic || !Traffic->GetRootComponent() || !CubeMesh) return;

    int32 PartIndex = 0;
    auto AddPart = [this, Traffic, &PartIndex](
        UStaticMesh* MeshAsset,
        const FString& SemanticName,
        const FVector& RelativeLocation,
        const FVector& RelativeScale,
        const FLinearColor& Color,
        const FRotator& RelativeRotation = FRotator::ZeroRotator) -> UStaticMeshComponent*
    {
        if (!MeshAsset) return nullptr;

        const FName ComponentName(*FString::Printf(
            TEXT("RITrafficPolish_%02d_%s"),
            PartIndex++,
            *SemanticName));

        UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(Traffic, ComponentName);
        if (!Part) return nullptr;

        Traffic->AddInstanceComponent(Part);
        Part->SetupAttachment(Traffic->GetRootComponent());
        Part->SetMobility(EComponentMobility::Movable);
        Part->SetStaticMesh(MeshAsset);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetCollisionProfileName(TEXT("NoCollision"));
        Part->SetGenerateOverlapEvents(false);
        Part->SetCanEverAffectNavigation(false);
        Part->SetRelativeLocation(RelativeLocation);
        Part->SetRelativeRotation(RelativeRotation);
        Part->SetRelativeScale3D(RelativeScale);
        Part->RegisterComponent();

        if (BaseMaterial)
        {
            if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Part))
            {
                Material->SetVectorParameterValue(TEXT("Color"), Color);
                Part->SetMaterial(0, Material);
            }
        }

        return Part;
    };

    const FString& Label = Traffic->GetTrafficLabel();
    const FLinearColor Glass(0.055f, 0.105f, 0.135f, 1.0f);
    const FLinearColor DarkTrim(0.035f, 0.040f, 0.045f, 1.0f);
    const FLinearColor Chrome(0.48f, 0.50f, 0.52f, 1.0f);
    const FLinearColor Plate(0.92f, 0.88f, 0.70f, 1.0f);
    const FLinearColor WarmLamp(1.0f, 0.82f, 0.32f, 1.0f);

    if (Label.Equals(TEXT("CNG AUTO"), ESearchCase::IgnoreCase))
    {
        // Compact three-wheeler details: dark windscreen, roof canopy, lower
        // front mask and tiny mirrors. These sit on the existing CNG silhouette;
        // the authoritative impact box remains unchanged.
        AddPart(CubeMesh, TEXT("CNG_Windscreen"), FVector(70.0f, 0.0f, 47.0f), FVector(0.055f, 0.70f, 0.42f), Glass, FRotator(0.0f, 0.0f, -8.0f));
        AddPart(CubeMesh, TEXT("CNG_Canopy"), FVector(-18.0f, 0.0f, 95.0f), FVector(1.30f, 0.96f, 0.10f), DarkTrim);
        AddPart(CubeMesh, TEXT("CNG_FrontMask"), FVector(91.0f, 0.0f, -5.0f), FVector(0.055f, 0.54f, 0.24f), FLinearColor(0.10f, 0.34f, 0.22f, 1.0f));
        AddPart(CubeMesh, TEXT("CNG_RearPlate"), FVector(-91.0f, 0.0f, -8.0f), FVector(0.035f, 0.25f, 0.10f), Plate);
        if (SphereMesh)
        {
            AddPart(SphereMesh, TEXT("CNG_MirrorL"), FVector(58.0f, -70.0f, 52.0f), FVector(0.10f, 0.055f, 0.08f), DarkTrim);
            AddPart(SphereMesh, TEXT("CNG_MirrorR"), FVector(58.0f, 70.0f, 52.0f), FVector(0.10f, 0.055f, 0.08f), DarkTrim);
        }
    }
    else
    {
        const bool bLarge =
            Label.Equals(TEXT("MICROBUS"), ESearchCase::IgnoreCase) ||
            Label.Equals(TEXT("DELIVERY VAN"), ESearchCase::IgnoreCase);
        const float FrontX = bLarge ? 167.0f : 143.0f;
        const float RearX = bLarge ? -168.0f : -143.0f;
        const float WindowZ = bLarge ? 55.0f : 46.0f;
        const float HalfWidth = bLarge ? 76.0f : 69.0f;

        AddPart(
            CubeMesh,
            TEXT("FrontGlass"),
            FVector(bLarge ? 70.0f : 52.0f, 0.0f, WindowZ),
            FVector(0.055f, bLarge ? 1.15f : 1.02f, bLarge ? 0.43f : 0.34f),
            Glass,
            FRotator(0.0f, 0.0f, -7.0f));

        AddPart(
            CubeMesh,
            TEXT("RearGlass"),
            FVector(bLarge ? -92.0f : -72.0f, 0.0f, WindowZ),
            FVector(0.045f, bLarge ? 1.12f : 0.98f, bLarge ? 0.36f : 0.29f),
            Glass,
            FRotator(0.0f, 0.0f, 5.0f));

        AddPart(CubeMesh, TEXT("FrontBumper"), FVector(FrontX, 0.0f, -30.0f), FVector(0.07f, bLarge ? 1.38f : 1.18f, 0.10f), Chrome);
        AddPart(CubeMesh, TEXT("RearBumper"), FVector(RearX, 0.0f, -30.0f), FVector(0.07f, bLarge ? 1.34f : 1.14f, 0.10f), DarkTrim);
        AddPart(CubeMesh, TEXT("FrontGrille"), FVector(FrontX + 2.0f, 0.0f, -4.0f), FVector(0.035f, bLarge ? 0.74f : 0.60f, 0.16f), DarkTrim);
        AddPart(CubeMesh, TEXT("RearPlate"), FVector(RearX - 2.0f, 0.0f, -5.0f), FVector(0.025f, 0.27f, 0.10f), Plate);

        if (SphereMesh)
        {
            AddPart(SphereMesh, TEXT("MirrorL"), FVector(48.0f, -HalfWidth - 9.0f, 40.0f), FVector(0.11f, 0.055f, 0.075f), DarkTrim);
            AddPart(SphereMesh, TEXT("MirrorR"), FVector(48.0f, HalfWidth + 9.0f, 40.0f), FVector(0.11f, 0.055f, 0.075f), DarkTrim);
        }

        if (Label.Equals(TEXT("TAXI"), ESearchCase::IgnoreCase))
        {
            AddPart(CubeMesh, TEXT("TaxiRoofSign"), FVector(-8.0f, 0.0f, 101.0f), FVector(0.42f, 0.24f, 0.14f), WarmLamp);
            AddPart(CubeMesh, TEXT("TaxiStripeL"), FVector(-5.0f, -76.0f, 4.0f), FVector(1.65f, 0.025f, 0.085f), DarkTrim);
            AddPart(CubeMesh, TEXT("TaxiStripeR"), FVector(-5.0f, 76.0f, 4.0f), FVector(1.65f, 0.025f, 0.085f), DarkTrim);
        }
        else if (Label.Equals(TEXT("MICROBUS"), ESearchCase::IgnoreCase))
        {
            for (int32 Window = 0; Window < 3; ++Window)
            {
                const float X = -78.0f + static_cast<float>(Window) * 72.0f;
                AddPart(CubeMesh, FString::Printf(TEXT("MicrobusWindowL%d"), Window), FVector(X, -79.0f, 48.0f), FVector(0.30f, 0.025f, 0.29f), Glass);
                AddPart(CubeMesh, FString::Printf(TEXT("MicrobusWindowR%d"), Window), FVector(X, 79.0f, 48.0f), FVector(0.30f, 0.025f, 0.29f), Glass);
            }
            AddPart(CubeMesh, TEXT("MicrobusBeltL"), FVector(-15.0f, -80.5f, -1.0f), FVector(1.82f, 0.018f, 0.075f), Chrome);
            AddPart(CubeMesh, TEXT("MicrobusBeltR"), FVector(-15.0f, 80.5f, -1.0f), FVector(1.82f, 0.018f, 0.075f), Chrome);
        }
        else if (Label.Equals(TEXT("DELIVERY VAN"), ESearchCase::IgnoreCase))
        {
            AddPart(CubeMesh, TEXT("VanSidePanelL"), FVector(-58.0f, -76.5f, 27.0f), FVector(0.80f, 0.022f, 0.48f), FLinearColor(0.88f, 0.84f, 0.66f, 1.0f));
            AddPart(CubeMesh, TEXT("VanSidePanelR"), FVector(-58.0f, 76.5f, 27.0f), FVector(0.80f, 0.022f, 0.48f), FLinearColor(0.88f, 0.84f, 0.66f, 1.0f));
            AddPart(CubeMesh, TEXT("VanRoofStrip"), FVector(-20.0f, 0.0f, 92.0f), FVector(1.58f, 1.20f, 0.045f), FLinearColor(0.72f, 0.74f, 0.76f, 1.0f));
        }
        else if (Label.Equals(TEXT("SUNDAY DRIVER"), ESearchCase::IgnoreCase))
        {
            AddPart(CubeMesh, TEXT("SundayRoofRailL"), FVector(-18.0f, -43.0f, 80.0f), FVector(0.82f, 0.035f, 0.035f), Chrome);
            AddPart(CubeMesh, TEXT("SundayRoofRailR"), FVector(-18.0f, 43.0f, 80.0f), FVector(0.82f, 0.035f, 0.035f), Chrome);
        }
        else if (Label.Equals(TEXT("LOST DRIVER"), ESearchCase::IgnoreCase))
        {
            AddPart(CubeMesh, TEXT("LostRoofBox"), FVector(-34.0f, 0.0f, 91.0f), FVector(0.62f, 0.56f, 0.15f), FLinearColor(0.18f, 0.20f, 0.22f, 1.0f));
        }
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI TRAFFIC VISUAL label=%s parts=%d collision=off"),
        *Label,
        PartIndex);
}
