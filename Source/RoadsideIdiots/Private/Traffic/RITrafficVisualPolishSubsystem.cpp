#include "Traffic/RITrafficVisualPolishSubsystem.h"

#include "Traffic/RITrafficVehicle.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
    UStaticMeshComponent* RIFindTrafficPart(ARITrafficVehicle* Traffic, const FName Name)
    {
        if (!Traffic) return nullptr;

        TArray<UStaticMeshComponent*> Components;
        Traffic->GetComponents<UStaticMeshComponent>(Components);
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

    UMaterialInterface* TrafficBodyMaterial = nullptr;
    if (UStaticMeshComponent* BodyVisual = RIFindTrafficPart(Traffic, FName(TEXT("BodyVisual"))))
    {
        TrafficBodyMaterial = BodyVisual->GetMaterial(0);
    }

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
        Part->SetCastShadow(false);
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

    auto AddBodyPart = [&AddPart, TrafficBodyMaterial](
        const FString& SemanticName,
        const FVector& RelativeLocation,
        const FVector& RelativeScale,
        const FRotator& RelativeRotation = FRotator::ZeroRotator) -> UStaticMeshComponent*
    {
        UStaticMeshComponent* Part = AddPart(
            LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")),
            SemanticName,
            RelativeLocation,
            RelativeScale,
            FLinearColor(0.35f, 0.35f, 0.35f, 1.0f),
            RelativeRotation);
        if (Part && TrafficBodyMaterial)
        {
            Part->SetMaterial(0, TrafficBodyMaterial);
        }
        return Part;
    };

    const FString& Label = Traffic->GetTrafficLabel();
    const FLinearColor Glass(0.035f, 0.075f, 0.095f, 1.0f);
    const FLinearColor DarkTrim(0.025f, 0.030f, 0.034f, 1.0f);
    const FLinearColor Chrome(0.40f, 0.43f, 0.46f, 1.0f);
    const FLinearColor Plate(0.92f, 0.88f, 0.70f, 1.0f);
    const FLinearColor WarmLamp(1.0f, 0.78f, 0.24f, 1.0f);
    const FLinearColor TailLamp(0.80f, 0.035f, 0.025f, 1.0f);

    if (Label.Equals(TEXT("CNG AUTO"), ESearchCase::IgnoreCase))
    {
        // Compact three-wheeler shell. All pieces are visual-only and use the
        // existing CNG actor as their parent; the impact box remains authoritative.
        AddPart(CubeMesh, TEXT("CNG_Windscreen"), FVector(70.0f, 0.0f, 47.0f), FVector(0.055f, 0.66f, 0.38f), Glass, FRotator(0.0f, 0.0f, -8.0f));
        AddPart(CubeMesh, TEXT("CNG_Canopy"), FVector(-18.0f, 0.0f, 95.0f), FVector(1.30f, 0.96f, 0.10f), DarkTrim);
        AddPart(CubeMesh, TEXT("CNG_FrontMask"), FVector(91.0f, 0.0f, -5.0f), FVector(0.055f, 0.54f, 0.24f), FLinearColor(0.10f, 0.34f, 0.22f, 1.0f));
        AddPart(CubeMesh, TEXT("CNG_RearPlate"), FVector(-91.0f, 0.0f, -8.0f), FVector(0.035f, 0.25f, 0.10f), Plate);
        AddPart(CubeMesh, TEXT("CNG_RearLampL"), FVector(-92.0f, -31.0f, 8.0f), FVector(0.025f, 0.13f, 0.11f), TailLamp);
        AddPart(CubeMesh, TEXT("CNG_RearLampR"), FVector(-92.0f, 31.0f, 8.0f), FVector(0.025f, 0.13f, 0.11f), TailLamp);
        AddPart(CubeMesh, TEXT("CNG_SideFrameL"), FVector(-8.0f, -54.0f, 48.0f), FVector(0.80f, 0.025f, 0.055f), DarkTrim);
        AddPart(CubeMesh, TEXT("CNG_SideFrameR"), FVector(-8.0f, 54.0f, 48.0f), FVector(0.80f, 0.025f, 0.055f), DarkTrim);
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
        const float WindowZ = bLarge ? 55.0f : 44.0f;
        const float HalfWidth = bLarge ? 76.0f : 69.0f;

        // Layer body-colored decks over the original low collision-safe block.
        // From chase-camera distance this produces a stepped hood/cabin/trunk
        // silhouette instead of one rectangular moving box.
        if (bLarge)
        {
            AddBodyPart(TEXT("LargeRoofCap"), FVector(-18.0f, 0.0f, 91.0f), FVector(1.65f, 1.24f, 0.055f));
        }
        else
        {
            AddBodyPart(TEXT("HoodDeck"), FVector(103.0f, 0.0f, 12.0f), FVector(0.72f, 1.32f, 0.14f));
            AddBodyPart(TEXT("TrunkDeck"), FVector(-111.0f, 0.0f, 15.0f), FVector(0.48f, 1.30f, 0.13f));
            AddBodyPart(TEXT("RoofCap"), FVector(-20.0f, 0.0f, 77.0f), FVector(1.14f, 1.08f, 0.055f));
        }

        AddPart(
            CubeMesh,
            TEXT("FrontGlass"),
            FVector(bLarge ? 70.0f : 48.0f, 0.0f, WindowZ),
            FVector(0.050f, bLarge ? 1.10f : 0.92f, bLarge ? 0.40f : 0.30f),
            Glass,
            FRotator(0.0f, 0.0f, -8.0f));

        AddPart(
            CubeMesh,
            TEXT("RearGlass"),
            FVector(bLarge ? -93.0f : -80.0f, 0.0f, WindowZ),
            FVector(0.040f, bLarge ? 1.05f : 0.82f, bLarge ? 0.34f : 0.24f),
            Glass,
            FRotator(0.0f, 0.0f, 6.0f));

        AddPart(CubeMesh, TEXT("FrontBumper"), FVector(FrontX, 0.0f, -30.0f), FVector(0.07f, bLarge ? 1.38f : 1.18f, 0.10f), Chrome);
        AddPart(CubeMesh, TEXT("RearBumper"), FVector(RearX, 0.0f, -30.0f), FVector(0.07f, bLarge ? 1.34f : 1.14f, 0.10f), DarkTrim);
        AddPart(CubeMesh, TEXT("FrontGrille"), FVector(FrontX + 2.0f, 0.0f, -4.0f), FVector(0.035f, bLarge ? 0.74f : 0.60f, 0.16f), DarkTrim);
        AddPart(CubeMesh, TEXT("RearPlate"), FVector(RearX - 2.0f, 0.0f, -5.0f), FVector(0.025f, 0.27f, 0.10f), Plate);

        // Lamps and side sills are strong vehicle cues at the exact distance
        // where the player is deciding whether to pass or brake.
        AddPart(CubeMesh, TEXT("HeadLampL"), FVector(FrontX + 3.0f, -45.0f, -5.0f), FVector(0.025f, 0.19f, 0.11f), WarmLamp);
        AddPart(CubeMesh, TEXT("HeadLampR"), FVector(FrontX + 3.0f, 45.0f, -5.0f), FVector(0.025f, 0.19f, 0.11f), WarmLamp);
        AddPart(CubeMesh, TEXT("TailLampL"), FVector(RearX - 3.0f, -44.0f, -5.0f), FVector(0.025f, 0.20f, 0.12f), TailLamp);
        AddPart(CubeMesh, TEXT("TailLampR"), FVector(RearX - 3.0f, 44.0f, -5.0f), FVector(0.025f, 0.20f, 0.12f), TailLamp);
        AddPart(CubeMesh, TEXT("SideSillL"), FVector(-10.0f, -HalfWidth - 1.0f, -27.0f), FVector(bLarge ? 1.65f : 1.38f, 0.022f, 0.055f), DarkTrim);
        AddPart(CubeMesh, TEXT("SideSillR"), FVector(-10.0f, HalfWidth + 1.0f, -27.0f), FVector(bLarge ? 1.65f : 1.38f, 0.022f, 0.055f), DarkTrim);

        if (!bLarge)
        {
            // Body-color rear window frame breaks up the old single gray cabin face.
            AddBodyPart(TEXT("RearPillarL"), FVector(-96.0f, -51.0f, 47.0f), FVector(0.045f, 0.095f, 0.32f));
            AddBodyPart(TEXT("RearPillarR"), FVector(-96.0f, 51.0f, 47.0f), FVector(0.045f, 0.095f, 0.32f));
            AddBodyPart(TEXT("RearHeader"), FVector(-96.0f, 0.0f, 70.0f), FVector(0.045f, 1.05f, 0.055f));
            AddPart(CubeMesh, TEXT("SideGlassL"), FVector(-28.0f, -63.0f, 45.0f), FVector(0.52f, 0.022f, 0.25f), Glass);
            AddPart(CubeMesh, TEXT("SideGlassR"), FVector(-28.0f, 63.0f, 45.0f), FVector(0.52f, 0.022f, 0.25f), Glass);
        }

        if (SphereMesh)
        {
            AddPart(SphereMesh, TEXT("MirrorL"), FVector(48.0f, -HalfWidth - 9.0f, 40.0f), FVector(0.11f, 0.055f, 0.075f), DarkTrim);
            AddPart(SphereMesh, TEXT("MirrorR"), FVector(48.0f, HalfWidth + 9.0f, 40.0f), FVector(0.11f, 0.055f, 0.075f), DarkTrim);
        }

        if (Label.Equals(TEXT("TAXI"), ESearchCase::IgnoreCase))
        {
            AddPart(CubeMesh, TEXT("TaxiRoofSign"), FVector(-8.0f, 0.0f, 91.0f), FVector(0.38f, 0.22f, 0.12f), WarmLamp);
            AddPart(CubeMesh, TEXT("TaxiStripeL"), FVector(-5.0f, -72.0f, 4.0f), FVector(1.55f, 0.020f, 0.070f), DarkTrim);
            AddPart(CubeMesh, TEXT("TaxiStripeR"), FVector(-5.0f, 72.0f, 4.0f), FVector(1.55f, 0.020f, 0.070f), DarkTrim);
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
            AddPart(CubeMesh, TEXT("VanSidePanelL"), FVector(-58.0f, -76.5f, 27.0f), FVector(0.80f, 0.022f, 0.48f), FLinearColor(0.72f, 0.70f, 0.56f, 1.0f));
            AddPart(CubeMesh, TEXT("VanSidePanelR"), FVector(-58.0f, 76.5f, 27.0f), FVector(0.80f, 0.022f, 0.48f), FLinearColor(0.72f, 0.70f, 0.56f, 1.0f));
            AddPart(CubeMesh, TEXT("VanRoofStrip"), FVector(-20.0f, 0.0f, 92.0f), FVector(1.58f, 1.20f, 0.045f), FLinearColor(0.60f, 0.62f, 0.64f, 1.0f));
        }
        else if (Label.Equals(TEXT("SUNDAY DRIVER"), ESearchCase::IgnoreCase))
        {
            AddPart(CubeMesh, TEXT("SundayRoofRailL"), FVector(-18.0f, -43.0f, 80.0f), FVector(0.82f, 0.035f, 0.035f), Chrome);
            AddPart(CubeMesh, TEXT("SundayRoofRailR"), FVector(-18.0f, 43.0f, 80.0f), FVector(0.82f, 0.035f, 0.035f), Chrome);
        }
        else if (Label.Equals(TEXT("LOST DRIVER"), ESearchCase::IgnoreCase))
        {
            AddPart(CubeMesh, TEXT("LostRoofBox"), FVector(-34.0f, 0.0f, 86.0f), FVector(0.56f, 0.50f, 0.12f), FLinearColor(0.14f, 0.16f, 0.18f, 1.0f));
        }
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI TRAFFIC VISUAL label=%s parts=%d style=layered_shell collision=off"),
        *Label,
        PartIndex);
}