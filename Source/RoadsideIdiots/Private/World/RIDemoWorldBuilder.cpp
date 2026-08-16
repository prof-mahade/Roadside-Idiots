#include "World/RIDemoWorldBuilder.h"
#include "Race/RIRaceManager.h"
#include "Race/RICheckpoint.h"
#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Core/RIRaceSettingsSubsystem.h"
#include "AI/RIAIController.h"
#include "AI/RIRacingLineFollower.h"
#include "Items/RIBananaPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/GameInstance.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
    URIRaceSettingsSubsystem* RIWorldBuilder_GetRaceSettings(UWorld* World)
    {
        if (!World) return nullptr;
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<URIRaceSettingsSubsystem>();
        }
        return nullptr;
    }
}

ARIDemoWorldBuilder::ARIDemoWorldBuilder()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARIDemoWorldBuilder::BuildWorld(ARIRaceManager* InRaceManager, APlayerController* PlayerController)
{
    CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!CubeMesh || !GetWorld() || !InRaceManager)
    {
        return;
    }

    BuildRoute();

    // One continuous flat collider under the entire prototype course. The
    // individual visible road boxes are intentionally non-colliding so Chaos
    // cannot catch the bike chassis on internal segment seams/overlaps.
    SpawnBox(FVector(0.0f, 0.0f, -40.0f), FRotator::ZeroRotator, FVector(300.0f, 300.0f, 0.8f));
    BuildTrackGeometry();
    BuildRoadsideIdentity();
    BuildCheckpoints(InRaceManager);
    SpawnPrototypePickups();
    SpawnRacers(InRaceManager, PlayerController);

    GetWorld()->SpawnActor<ASkyAtmosphere>();

    if (ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-48.0f, -28.0f, 0.0f)))
    {
        // ADirectionalLight::GetComponent() is not available to the packaged
        // game target in UE 5.8. Use the runtime ALight accessor instead and
        // cast the returned light component to the directional subtype.
        if (UDirectionalLightComponent* SunComponent = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
        {
            SunComponent->SetIntensity(8.0f);
            SunComponent->SetAtmosphereSunLight(true);
        }
    }
    if (ASkyLight* Sky = GetWorld()->SpawnActor<ASkyLight>())
    {
        Sky->GetLightComponent()->SetIntensity(1.2f);
        Sky->GetLightComponent()->SetRealTimeCapture(false);
    }
}

void ARIDemoWorldBuilder::BuildRoute()
{
    RoutePoints.Reset();

    // VPR-24B: double the route resolution. The course is the exact same oval,
    // but the path tangent and physical barrier chain change in smaller steps,
    // which is much friendlier to a high-speed physics bike.
    constexpr int32 PointCount = 80;
    constexpr float RadiusX = 9000.0f;
    constexpr float RadiusY = 5000.0f;

    for (int32 Index = 0; Index < PointCount; ++Index)
    {
        const float Angle = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(PointCount);
        RoutePoints.Add(FVector(FMath::Cos(Angle) * RadiusX, FMath::Sin(Angle) * RadiusY, 85.0f));
    }
}

AStaticMeshActor* ARIDemoWorldBuilder::SpawnBox(const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    if (!GetWorld() || !CubeMesh)
    {
        return nullptr;
    }

    AStaticMeshActor* Box = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (!Box)
    {
        return nullptr;
    }

    UStaticMeshComponent* Mesh = Box->GetStaticMeshComponent();
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(CubeMesh);
    Mesh->SetCollisionProfileName(TEXT("BlockAll"));
    Mesh->SetGenerateOverlapEvents(false);
    Box->SetActorScale3D(Scale);
    return Box;
}

void ARIDemoWorldBuilder::BuildTrackGeometry()
{
    const int32 Count = RoutePoints.Num();
    if (Count < 3) return;

    constexpr float RoadSegmentPadding = 120.0f;
    constexpr float BarrierSegmentPadding = 160.0f;

    for (int32 Index = 0; Index < Count; ++Index)
    {
        const FVector A = RoutePoints[Index];
        const FVector B = RoutePoints[(Index + 1) % Count];
        FVector Direction = B - A;
        Direction.Z = 0.0f;
        const float Length = Direction.Size();
        if (Length < 1.0f) continue;

        const FVector Forward = Direction / Length;
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const FVector Center = (A + B) * 0.5f;
        const FRotator Rotation = Forward.Rotation();

        // Visual road only. Keep it 1 cm above the seamless collision floor to
        // avoid z-fighting, but do not let its overlapping boxes participate in
        // physics.
        if (AStaticMeshActor* Road = SpawnBox(
            FVector(Center.X, Center.Y, -9.0f),
            Rotation,
            FVector((Length + RoadSegmentPadding) / 100.0f, RoadWidth / 100.0f, 0.20f)))
        {
            if (UStaticMeshComponent* RoadMesh = Road->GetStaticMeshComponent())
            {
                RoadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                RoadMesh->SetCollisionProfileName(TEXT("NoCollision"));
            }
        }

        const float BarrierOffset = RoadWidth * 0.5f + 28.0f;
        const FVector LeftBarrier = Center - Right * BarrierOffset;
        const FVector RightBarrier = Center + Right * BarrierOffset;

        // More, shorter barrier pieces with less overhang reduce the inward
        // corner wedges created by the old coarse 40-piece chain.
        SpawnBox(
            FVector(LeftBarrier.X, LeftBarrier.Y, 60.0f),
            Rotation,
            FVector((Length + BarrierSegmentPadding) / 100.0f, 0.65f, 1.20f));

        SpawnBox(
            FVector(RightBarrier.X, RightBarrier.Y, 60.0f),
            Rotation,
            FVector((Length + BarrierSegmentPadding) / 100.0f, 0.65f, 1.20f));
    }
}

void ARIDemoWorldBuilder::BuildRoadsideIdentity()
{
    if (!GetWorld() || !CubeMesh || RoutePoints.Num() < 16) return;

    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(
        nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    auto SpawnDecoration = [this, BaseMaterial](
        const FVector& Location,
        const FRotator& Rotation,
        const FVector& Scale,
        const FLinearColor& Color) -> AStaticMeshActor*
    {
        AStaticMeshActor* Actor = SpawnBox(Location, Rotation, Scale);
        if (!Actor) return nullptr;

        if (UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent())
        {
            // Roadside identity is presentation only. Keeping every decorative
            // piece non-colliding guarantees this pass cannot change the frozen
            // bike/road/AI handling baseline.
            Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Mesh->SetCollisionProfileName(TEXT("NoCollision"));

            if (BaseMaterial)
            {
                if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Mesh))
                {
                    Material->SetVectorParameterValue(TEXT("Color"), Color);
                    Mesh->SetMaterial(0, Material);
                }
            }
        }
        return Actor;
    };

    auto GetRouteFrame = [this](const int32 Index, FVector& OutForward, FVector& OutOutward)
    {
        const int32 Count = RoutePoints.Num();
        const int32 PrevIndex = (Index - 1 + Count) % Count;
        const int32 NextIndex = (Index + 1) % Count;
        OutForward = (RoutePoints[NextIndex] - RoutePoints[PrevIndex]).GetSafeNormal2D();
        OutOutward = RoutePoints[Index].GetSafeNormal2D();
    };

    const FLinearColor PoleColor(0.20f, 0.18f, 0.15f, 1.0f);
    const FLinearColor WireColor(0.055f, 0.055f, 0.05f, 1.0f);
    const FLinearColor TrunkColor(0.24f, 0.14f, 0.07f, 1.0f);
    const FLinearColor LeafColor(0.10f, 0.34f, 0.10f, 1.0f);

    // A sparse utility line gives the lap a recognizable roadside rhythm and
    // makes speed easier to perceive. Ten poles are enough to read as a place
    // without turning the prototype into a prop forest.
    TArray<FVector> WirePoints;
    for (int32 Index = 0; Index < RoutePoints.Num(); Index += 8)
    {
        FVector Forward;
        FVector Outward;
        GetRouteFrame(Index, Forward, Outward);

        FVector PoleBase = RoutePoints[Index] + Outward * (RoadWidth * 0.5f + 760.0f);
        PoleBase.Z = 0.0f;

        SpawnDecoration(
            PoleBase + FVector::UpVector * 245.0f,
            FRotator::ZeroRotator,
            FVector(0.13f, 0.13f, 4.90f),
            PoleColor);

        SpawnDecoration(
            PoleBase + FVector::UpVector * 455.0f,
            Outward.Rotation(),
            FVector(0.82f, 0.09f, 0.09f),
            PoleColor);

        WirePoints.Add(PoleBase + FVector::UpVector * 478.0f);
    }

    for (int32 Index = 0; Index < WirePoints.Num(); ++Index)
    {
        const FVector A = WirePoints[Index];
        const FVector B = WirePoints[(Index + 1) % WirePoints.Num()];
        FVector Direction = B - A;
        const float Length = Direction.Size();
        if (Length < 1.0f) continue;

        SpawnDecoration(
            (A + B) * 0.5f,
            Direction.Rotation(),
            FVector(Length / 100.0f, 0.035f, 0.035f),
            WireColor);
    }

    // Lightweight tea-stall / roadside-shop silhouettes. These are deliberately
    // clean, colorful and affectionate rather than poverty caricatures. They are
    // far outside the race corridor and exist only to make the course feel less
    // like a generic test oval.
    const int32 StallIndices[] = {6, 25, 44, 63};
    const FLinearColor StallWalls[] = {
        FLinearColor(0.82f, 0.37f, 0.12f, 1.0f),
        FLinearColor(0.16f, 0.48f, 0.62f, 1.0f),
        FLinearColor(0.67f, 0.18f, 0.25f, 1.0f),
        FLinearColor(0.18f, 0.55f, 0.30f, 1.0f)};
    const FLinearColor RoofColors[] = {
        FLinearColor(0.12f, 0.28f, 0.42f, 1.0f),
        FLinearColor(0.52f, 0.16f, 0.10f, 1.0f),
        FLinearColor(0.12f, 0.36f, 0.24f, 1.0f),
        FLinearColor(0.48f, 0.31f, 0.09f, 1.0f)};

    for (int32 Stall = 0; Stall < UE_ARRAY_COUNT(StallIndices); ++Stall)
    {
        const int32 Index = StallIndices[Stall] % RoutePoints.Num();
        FVector Forward;
        FVector Outward;
        GetRouteFrame(Index, Forward, Outward);
        const FRotator Rotation = Forward.Rotation();

        FVector Center = RoutePoints[Index] + Outward * (RoadWidth * 0.5f + 1450.0f);
        Center.Z = 0.0f;
        const FVector RoadFacing = -Outward;

        SpawnDecoration(
            Center + FVector::UpVector * 135.0f,
            Rotation,
            FVector(3.8f, 2.6f, 2.7f),
            StallWalls[Stall]);

        SpawnDecoration(
            Center + FVector::UpVector * 292.0f,
            Rotation,
            FVector(4.25f, 3.05f, 0.16f),
            RoofColors[Stall]);

        SpawnDecoration(
            Center + RoadFacing * 170.0f + FVector::UpVector * 82.0f,
            Rotation,
            FVector(2.20f, 0.48f, 0.82f),
            FLinearColor(0.38f, 0.19f, 0.08f, 1.0f));

        SpawnDecoration(
            Center + RoadFacing * 205.0f + FVector::UpVector * 224.0f,
            Rotation,
            FVector(3.10f, 1.15f, 0.12f),
            RoofColors[Stall]);

        SpawnDecoration(
            Center + RoadFacing * 148.0f + FVector::UpVector * 350.0f,
            Rotation,
            FVector(1.65f, 0.13f, 0.48f),
            FLinearColor(0.92f, 0.78f, 0.24f, 1.0f));

        SpawnDecoration(
            Center + RoadFacing * 345.0f + Forward * 155.0f + FVector::UpVector * 36.0f,
            Rotation,
            FVector(1.30f, 0.34f, 0.36f),
            FLinearColor(0.32f, 0.18f, 0.09f, 1.0f));

        SpawnDecoration(
            Center + RoadFacing * 345.0f - Forward * 155.0f + FVector::UpVector * 36.0f,
            Rotation,
            FVector(1.30f, 0.34f, 0.36f),
            FLinearColor(0.32f, 0.18f, 0.09f, 1.0f));
    }

    // Small roadside signboards provide landmarks around the lap. No generated
    // text is used; the color blocks are intentional prototype placeholders.
    const int32 SignIndices[] = {14, 34, 54, 74};
    const FLinearColor SignColors[] = {
        FLinearColor(0.10f, 0.48f, 0.72f, 1.0f),
        FLinearColor(0.86f, 0.45f, 0.08f, 1.0f),
        FLinearColor(0.16f, 0.62f, 0.34f, 1.0f),
        FLinearColor(0.68f, 0.16f, 0.24f, 1.0f)};

    for (int32 Sign = 0; Sign < UE_ARRAY_COUNT(SignIndices); ++Sign)
    {
        const int32 Index = SignIndices[Sign] % RoutePoints.Num();
        FVector Forward;
        FVector Outward;
        GetRouteFrame(Index, Forward, Outward);

        FVector Base = RoutePoints[Index] + Outward * (RoadWidth * 0.5f + 990.0f);
        Base.Z = 0.0f;

        SpawnDecoration(
            Base + FVector::UpVector * 150.0f,
            FRotator::ZeroRotator,
            FVector(0.10f, 0.10f, 3.0f),
            PoleColor);

        SpawnDecoration(
            Base + FVector::UpVector * 292.0f,
            Forward.Rotation(),
            FVector(1.75f, 0.14f, 0.62f),
            SignColors[Sign]);
    }

    // Simple tropical tree silhouettes fill only selected empty stretches. Their
    // large offset preserves sight lines for traffic, pickups and rival attacks.
    for (int32 Index = 3; Index < RoutePoints.Num(); Index += 7)
    {
        FVector Forward;
        FVector Outward;
        GetRouteFrame(Index, Forward, Outward);

        FVector TreeBase = RoutePoints[Index] + Outward * (RoadWidth * 0.5f + 2150.0f);
        TreeBase.Z = 0.0f;

        const float HeightVariation = 0.88f + 0.10f * static_cast<float>(Index % 3);
        SpawnDecoration(
            TreeBase + FVector::UpVector * (155.0f * HeightVariation),
            FRotator::ZeroRotator,
            FVector(0.24f, 0.24f, 3.10f * HeightVariation),
            TrunkColor);

        SpawnDecoration(
            TreeBase + FVector::UpVector * (350.0f * HeightVariation),
            FRotator(0.0f, static_cast<float>(Index * 17), 0.0f),
            FVector(2.1f, 1.7f, 1.25f) * HeightVariation,
            LeafColor);
    }
}

void ARIDemoWorldBuilder::BuildCheckpoints(ARIRaceManager* RaceManager)
{
    if (!GetWorld() || RoutePoints.Num() < 16 || !RaceManager) return;

    // Keep the same eight checkpoint locations by expressing them as fractions
    // of the route rather than depending on the old 40-point resolution.
    TArray<int32> CheckpointRouteIndices;
    CheckpointRouteIndices.Reserve(8);
    for (int32 Checkpoint = 1; Checkpoint < 8; ++Checkpoint)
    {
        CheckpointRouteIndices.Add((RoutePoints.Num() * Checkpoint) / 8);
    }
    CheckpointRouteIndices.Add(0);

    const URIRaceSettingsSubsystem* Settings = RIWorldBuilder_GetRaceSettings(GetWorld());
    const int32 TotalLaps = Settings ? Settings->GetLapCount() : 3;
    RaceManager->ConfigureRace(CheckpointRouteIndices.Num(), TotalLaps);

    for (int32 CheckpointIndex = 0; CheckpointIndex < CheckpointRouteIndices.Num(); ++CheckpointIndex)
    {
        const int32 RouteIndex = CheckpointRouteIndices[CheckpointIndex];
        const int32 NextRouteIndex = (RouteIndex + 1) % RoutePoints.Num();
        FVector Forward = (RoutePoints[NextRouteIndex] - RoutePoints[RouteIndex]).GetSafeNormal2D();
        const FVector Location = RoutePoints[RouteIndex] + FVector(0.0f, 0.0f, 80.0f);
        const FRotator Rotation = Forward.Rotation();

        if (ARICheckpoint* Checkpoint = GetWorld()->SpawnActor<ARICheckpoint>(Location, Rotation))
        {
            Checkpoint->Configure(RaceManager, CheckpointIndex, FVector(135.0f, RoadWidth * 0.48f, 200.0f));
        }
    }
}

void ARIDemoWorldBuilder::SpawnPrototypePickups()
{
    if (!GetWorld() || RoutePoints.Num() < 16) return;

    const float PickupFractions[] = {0.10f, 0.225f, 0.35f, 0.475f, 0.60f, 0.725f, 0.85f, 0.975f};
    const float LaneOffsets[] = {-250.0f, 180.0f, 0.0f, -190.0f, 255.0f, 0.0f, -245.0f, 210.0f};

    for (int32 PickupIndex = 0; PickupIndex < UE_ARRAY_COUNT(PickupFractions); ++PickupIndex)
    {
        const int32 RouteIndex = FMath::RoundToInt(PickupFractions[PickupIndex] * static_cast<float>(RoutePoints.Num())) % RoutePoints.Num();
        const int32 PrevIndex = (RouteIndex - 1 + RoutePoints.Num()) % RoutePoints.Num();
        const int32 NextIndex = (RouteIndex + 1) % RoutePoints.Num();
        const FVector Tangent = (RoutePoints[NextIndex] - RoutePoints[PrevIndex]).GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();

        FVector Location = RoutePoints[RouteIndex] + Right * LaneOffsets[PickupIndex];
        Location.Z = 42.0f;

        GetWorld()->SpawnActor<ARIBananaPickup>(Location, Tangent.Rotation());
    }
}

void ARIDemoWorldBuilder::SpawnRacers(ARIRaceManager* RaceManager, APlayerController* PlayerController)
{
    if (!GetWorld() || RoutePoints.Num() < 4 || !RaceManager) return;

    const URIRaceSettingsSubsystem* Settings = RIWorldBuilder_GetRaceSettings(GetWorld());
    const int32 OpponentCount = Settings ? FMath::Clamp(Settings->GetOpponentCount(), 2, 6) : 3;
    const int32 RacerCount = OpponentCount + 1;

    const FVector StartBase = RoutePoints[1];
    const FVector Forward = (RoutePoints[2] - RoutePoints[1]).GetSafeNormal2D();
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
    const FRotator StartRotation = Forward.Rotation();

    // The physical start grid remains wide for readable staging, but it is not
    // reused as a permanent racing lane.
    const float GridLaneOffsets[3] = {0.0f, -300.0f, 300.0f};
    const float RaceLaneOffsets[6] = {-220.0f, 220.0f, -40.0f, -130.0f, 130.0f, 40.0f};
    constexpr float RowSpacing = 330.0f;

    for (int32 RacerIndex = 0; RacerIndex < RacerCount; ++RacerIndex)
    {
        const int32 Row = RacerIndex / 3;
        const int32 Column = RacerIndex % 3;
        const float GridLaneOffset = GridLaneOffsets[Column];

        FVector Location = StartBase - Forward * (static_cast<float>(Row) * RowSpacing) + Right * GridLaneOffset;
        Location.Z = 28.0f;

        ARIBikePawn* Bike = GetWorld()->SpawnActor<ARIBikePawn>(Location, StartRotation);
        if (!Bike) continue;

        Bike->SetRecoveryTransform(FTransform(StartRotation, Location));

        const bool bHuman = RacerIndex == 0;
        const FName ParticipantId = bHuman
            ? FName(TEXT("PLAYER"))
            : FName(*FString::Printf(TEXT("BOT_%02d"), RacerIndex));
        Bike->GetParticipantComponent()->AssignParticipant(ParticipantId, bHuman);
        RaceManager->RegisterParticipant(ParticipantId);

        if (bHuman)
        {
            if (PlayerController)
            {
                PlayerController->Possess(Bike);
            }
        }
        else
        {
            ARIAIController* AI = GetWorld()->SpawnActor<ARIAIController>();
            if (AI)
            {
                AI->Possess(Bike);
                AI->SetActorTickInterval(0.0f);

                const int32 RivalIndex = FMath::Clamp(RacerIndex - 1, 0, 5);
                const float RaceLane = RaceLaneOffsets[RivalIndex];
                AI->SetRoute(RoutePoints, 2, RaceLane);

                // VPR-24E: high-level AI still decides pace, items, grudges and
                // chaos. A separate low-level racing driver owns the final
                // steering command. Tick order is explicitly:
                //     chaos AI -> racing line follower -> bike physics.
                // This prevents five competing steering corrections from
                // producing the left/right wall ping-pong seen in VPR-24B-D.
                if (ARIRacingLineFollower* Driver = GetWorld()->SpawnActor<ARIRacingLineFollower>())
                {
                    Driver->Configure(Bike, RoutePoints, RaceLane);
                    Driver->AddTickPrerequisiteActor(AI);

                    if (URIBikeMovementComponent* Movement = Bike->GetBikeMovement())
                    {
                        Movement->AddTickPrerequisiteActor(AI);
                        Movement->AddTickPrerequisiteActor(Driver);
                    }
                }
                else if (URIBikeMovementComponent* Movement = Bike->GetBikeMovement())
                {
                    Movement->AddTickPrerequisiteActor(AI);
                }
            }
        }
    }
}
