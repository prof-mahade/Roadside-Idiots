#include "World/RIDemoWorldBuilder.h"
#include "Race/RIRaceManager.h"
#include "Race/RICheckpoint.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIParticipantComponent.h"
#include "AI/RIAIController.h"
#include "Items/RIBananaPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "GameFramework/PlayerController.h"

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
    BuildCheckpoints(InRaceManager);
    SpawnPrototypePickups();
    SpawnRacers(InRaceManager, PlayerController);

    GetWorld()->SpawnActor<ASkyAtmosphere>();

    if (ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-48.0f, -28.0f, 0.0f)))
    {
        if (UDirectionalLightComponent* SunComponent = Sun->GetComponent())
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
    constexpr int32 PointCount = 40;
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
    constexpr float BarrierSegmentPadding = 240.0f;

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

void ARIDemoWorldBuilder::BuildCheckpoints(ARIRaceManager* RaceManager)
{
    if (!GetWorld() || RoutePoints.Num() < 10 || !RaceManager) return;

    TArray<int32> CheckpointRouteIndices = {5, 10, 15, 20, 25, 30, 35, 0};
    RaceManager->ConfigureCheckpoints(CheckpointRouteIndices.Num());

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
    if (!GetWorld() || RoutePoints.Num() < 8) return;

    const int32 PickupIndices[] = {4, 9, 14, 19, 24, 29, 34, 39};
    const float LaneOffsets[] = {-250.0f, 180.0f, 0.0f, -190.0f, 255.0f, 0.0f, -245.0f, 210.0f};

    for (int32 PickupIndex = 0; PickupIndex < UE_ARRAY_COUNT(PickupIndices); ++PickupIndex)
    {
        const int32 RouteIndex = PickupIndices[PickupIndex] % RoutePoints.Num();
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

    const FVector StartBase = RoutePoints[1];
    const FVector Forward = (RoutePoints[2] - RoutePoints[1]).GetSafeNormal2D();
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
    const FRotator StartRotation = Forward.Rotation();
    const float LaneOffsets[4] = {-315.0f, -105.0f, 105.0f, 315.0f};

    for (int32 RacerIndex = 0; RacerIndex < 4; ++RacerIndex)
    {
        FVector Location = StartBase - Forward * (RacerIndex * 260.0f) + Right * LaneOffsets[RacerIndex];
        Location.Z = 28.0f;

        ARIBikePawn* Bike = GetWorld()->SpawnActor<ARIBikePawn>(Location, StartRotation);
        if (!Bike) continue;

        Bike->SetRecoveryTransform(FTransform(StartRotation, Location));

        const bool bHuman = RacerIndex == 0;
        const FName ParticipantId = bHuman ? FName(TEXT("PLAYER")) : FName(*FString::Printf(TEXT("BOT_%02d"), RacerIndex));
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
                AI->SetRoute(RoutePoints, 2, LaneOffsets[RacerIndex]);
            }
        }
    }
}
