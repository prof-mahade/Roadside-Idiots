#include "Presentation/RIFinishCelebrationSubsystem.h"

#include "Core/RIParticipantComponent.h"
#include "Race/RIRaceManager.h"
#include "Vehicle/RIBikePawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

bool URIFinishCelebrationSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld() && (!bCelebrated || ConfettiPieces.Num() > 0);
}

TStatId URIFinishCelebrationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIFinishCelebrationSubsystem, STATGROUP_Tickables);
}

ARIBikePawn* URIFinishCelebrationSubsystem::FindHumanBike() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        ARIBikePawn* Bike = *It;
        const URIParticipantComponent* Participant = Bike ? Bike->GetParticipantComponent() : nullptr;
        if (Bike && Participant && Participant->IsHumanControlled())
        {
            return Bike;
        }
    }
    return nullptr;
}

ARIRaceManager* URIFinishCelebrationSubsystem::FindRaceManager() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    for (TActorIterator<ARIRaceManager> It(World); It; ++It)
    {
        if (*It) return *It;
    }
    return nullptr;
}

void URIFinishCelebrationSubsystem::Tick(const float DeltaTime)
{
    UpdateConfetti(FMath::Max(0.0f, DeltaTime));

    if (bCelebrated) return;

    ARIBikePawn* HumanBike = FindHumanBike();
    ARIRaceManager* RaceManager = FindRaceManager();
    if (!HumanBike || !RaceManager || !HumanBike->GetParticipantComponent()) return;

    FRIRaceProgress Progress;
    if (!RaceManager->GetProgress(HumanBike->GetParticipantComponent()->GetParticipantId(), Progress) || !Progress.bFinished)
    {
        return;
    }

    SpawnCelebration(
        HumanBike->GetActorLocation() + FVector::UpVector * 230.0f,
        HumanBike->GetActorForwardVector().GetSafeNormal2D());
    bCelebrated = true;
}

void URIFinishCelebrationSubsystem::UpdateConfetti(const float DeltaTime)
{
    if (DeltaTime <= 0.0f || ConfettiPieces.Num() == 0) return;

    const FVector GravityLikeAcceleration(0.0f, 0.0f, -470.0f);

    for (int32 Index = ConfettiPieces.Num() - 1; Index >= 0; --Index)
    {
        FRIConfettiPiece& Piece = ConfettiPieces[Index];
        AStaticMeshActor* Actor = Piece.Actor.Get();
        if (!Actor)
        {
            ConfettiPieces.RemoveAtSwap(Index, 1, EAllowShrinking::No);
            continue;
        }

        Piece.AgeSeconds += DeltaTime;
        if (Piece.AgeSeconds >= Piece.LifetimeSeconds)
        {
            Actor->Destroy();
            ConfettiPieces.RemoveAtSwap(Index, 1, EAllowShrinking::No);
            continue;
        }

        // Kinematic confetti keeps collision genuinely disabled. A light flutter
        // and drag gives enough motion for a prototype celebration without asking
        // Chaos to simulate a body that intentionally has no collision shape.
        Piece.Velocity += GravityLikeAcceleration * DeltaTime;
        const float Flutter = FMath::Sin(Piece.AgeSeconds * 9.0f + Piece.FlutterPhase);
        Piece.Velocity.X += Flutter * 22.0f * DeltaTime;
        Piece.Velocity.Y += FMath::Cos(Piece.AgeSeconds * 7.5f + Piece.FlutterPhase) * 26.0f * DeltaTime;
        Piece.Velocity *= FMath::Clamp(1.0f - 0.30f * DeltaTime, 0.0f, 1.0f);

        Actor->AddActorWorldOffset(Piece.Velocity * DeltaTime, false, nullptr, ETeleportType::TeleportPhysics);
        Actor->AddActorLocalRotation(Piece.AngularVelocityDegrees * DeltaTime, false, nullptr, ETeleportType::TeleportPhysics);
    }
}

void URIFinishCelebrationSubsystem::SpawnCelebration(const FVector& Origin, const FVector& Forward)
{
    UWorld* World = GetWorld();
    if (!World) return;

    ConfettiMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    BaseMaterial = LoadObject<UMaterialInterface>(
        nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!ConfettiMesh) return;

    const FVector SafeForward = Forward.IsNearlyZero() ? FVector::ForwardVector : Forward.GetSafeNormal2D();
    const FVector Right = FVector::CrossProduct(FVector::UpVector, SafeForward).GetSafeNormal2D();

    const FLinearColor Colors[] = {
        FLinearColor(1.0f, 0.70f, 0.05f, 1.0f),
        FLinearColor(0.08f, 0.72f, 0.62f, 1.0f),
        FLinearColor(0.94f, 0.20f, 0.16f, 1.0f),
        FLinearColor(0.30f, 0.62f, 1.0f, 1.0f),
        FLinearColor(0.80f, 0.28f, 0.92f, 1.0f),
        FLinearColor(0.96f, 0.96f, 0.90f, 1.0f)};

    constexpr int32 PieceCount = 30;
    ConfettiPieces.Reserve(ConfettiPieces.Num() + PieceCount);

    for (int32 Index = 0; Index < PieceCount; ++Index)
    {
        const float Side = FMath::FRandRange(-1.0f, 1.0f);
        const float Along = FMath::FRandRange(-0.35f, 0.65f);
        const FVector SpawnLocation =
            Origin +
            Right * (Side * FMath::FRandRange(60.0f, 250.0f)) +
            SafeForward * (Along * 180.0f) +
            FVector::UpVector * FMath::FRandRange(-30.0f, 120.0f);

        AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(
            SpawnLocation,
            FRotator(
                FMath::FRandRange(-35.0f, 35.0f),
                FMath::FRandRange(0.0f, 360.0f),
                FMath::FRandRange(-35.0f, 35.0f)));
        if (!Actor) continue;

        Actor->SetActorEnableCollision(false);
        Actor->SetActorScale3D(FVector(
            FMath::FRandRange(0.08f, 0.16f),
            FMath::FRandRange(0.035f, 0.075f),
            FMath::FRandRange(0.015f, 0.035f)));

        UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
        if (!Mesh)
        {
            Actor->Destroy();
            continue;
        }

        Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetStaticMesh(ConfettiMesh);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetCollisionProfileName(TEXT("NoCollision"));
        Mesh->SetGenerateOverlapEvents(false);
        Mesh->SetCanEverAffectNavigation(false);
        Mesh->SetSimulatePhysics(false);
        Mesh->SetEnableGravity(false);

        if (BaseMaterial)
        {
            if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Mesh))
            {
                Material->SetVectorParameterValue(TEXT("Color"), Colors[Index % UE_ARRAY_COUNT(Colors)]);
                Mesh->SetMaterial(0, Material);
            }
        }

        FRIConfettiPiece& Piece = ConfettiPieces.AddDefaulted_GetRef();
        Piece.Actor = Actor;
        Piece.Velocity =
            SafeForward * FMath::FRandRange(45.0f, 155.0f) +
            Right * FMath::FRandRange(-240.0f, 240.0f) +
            FVector::UpVector * FMath::FRandRange(310.0f, 620.0f);
        Piece.AngularVelocityDegrees = FRotator(
            FMath::FRandRange(-380.0f, 380.0f),
            FMath::FRandRange(-520.0f, 520.0f),
            FMath::FRandRange(-620.0f, 620.0f));
        Piece.LifetimeSeconds = FMath::FRandRange(2.8f, 4.2f);
        Piece.FlutterPhase = FMath::FRandRange(0.0f, 2.0f * PI);
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI FINISH CELEBRATION confetti=%d collision=off simulation=kinematic"),
        ConfettiPieces.Num());
}
