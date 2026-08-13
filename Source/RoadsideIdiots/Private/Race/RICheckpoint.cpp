#include "Race/RICheckpoint.h"
#include "Race/RIRaceManager.h"
#include "Core/RIParticipantComponent.h"
#include "Components/BoxComponent.h"

ARICheckpoint::ARICheckpoint()
{
    PrimaryActorTick.bCanEverTick = false;
    Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
    SetRootComponent(Trigger);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Trigger->SetGenerateOverlapEvents(true);
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &ARICheckpoint::HandleOverlap);
}

void ARICheckpoint::Configure(ARIRaceManager* InRaceManager, int32 InCheckpointIndex, const FVector& BoxExtent)
{
    RaceManager = InRaceManager;
    CheckpointIndex = InCheckpointIndex;
    Trigger->SetBoxExtent(BoxExtent);
}

void ARICheckpoint::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!RaceManager || !OtherActor) return;
    if (URIParticipantComponent* Participant = OtherActor->FindComponentByClass<URIParticipantComponent>())
    {
        RaceManager->ReportCheckpoint(Participant->GetParticipantId(), CheckpointIndex);
    }
}
