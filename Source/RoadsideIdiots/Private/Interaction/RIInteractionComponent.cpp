#include "Interaction/RIInteractionComponent.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "AI/RIAIController.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Audio/RIAudioEvents.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

URIInteractionComponent::URIInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool URIInteractionComponent::TrySideInteraction(float Side)
{
    ARIBikePawn* OwnerBike = Cast<ARIBikePawn>(GetOwner());
    if (!OwnerBike || !OwnerBike->AreRaceControlsEnabled() || !GetWorld())
    {
        return false;
    }

    const double Now = GetWorld()->GetTimeSeconds();
    if (Now - LastUseTime < CooldownSeconds)
    {
        return false;
    }
    LastUseTime = Now;

    Side = Side < 0.0f ? -1.0f : 1.0f;
    RIPrototypeVisuals::PlaySideAction(OwnerBike, Side);

    const FVector SideDirection = OwnerBike->GetActorRightVector() * Side;
    const FVector Start = OwnerBike->GetActorLocation() + OwnerBike->GetActorForwardVector() * 35.0f;
    const FVector End = Start + SideDirection * Reach;

    FCollisionObjectQueryParams ObjectQuery;
    ObjectQuery.AddObjectTypesToQuery(ECC_PhysicsBody);
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RISideInteraction), false, OwnerBike);
    TArray<FHitResult> Hits;

    const bool bHitAnything = GetWorld()->SweepMultiByObjectType(
        Hits,
        Start,
        End,
        FQuat::Identity,
        ObjectQuery,
        FCollisionShape::MakeSphere(Radius),
        QueryParams);

    if (!bHitAnything)
    {
        return false;
    }

    for (const FHitResult& Hit : Hits)
    {
        ARIBikePawn* OtherBike = Cast<ARIBikePawn>(Hit.GetActor());
        if (!OtherBike || OtherBike == OwnerBike || !OtherBike->AreRaceControlsEnabled())
        {
            continue;
        }

        if (UStaticMeshComponent* OtherChassis = OtherBike->GetChassis())
        {
            OtherChassis->AddImpulse(SideDirection * SideVelocityChange + FVector::UpVector * 38.0f, NAME_None, true);

            const FVector RollKick = OtherBike->GetActorForwardVector() * (-Side * 3.6f);
            const FVector YawKick = FVector::UpVector * (Side * 0.9f);
            OtherChassis->AddAngularImpulseInRadians(RollKick + YawKick, NAME_None, true);
        }

        if (URIHealthComponent* OtherHealth = OtherBike->GetHealthComponent())
        {
            OtherHealth->ApplyImpactFromSource(ImpactCost, FName(TEXT("Slap")));
        }

        RIPrototypeVisuals::PlayReaction(OtherBike, -Side);
        RIAudioEvents::Play(this, TEXT("SlapHit"), OtherBike->GetActorLocation(), 1.0f, FMath::FRandRange(0.94f, 1.06f));

        ARIAIController* RivalController = Cast<ARIAIController>(OtherBike->GetController());
        if (RivalController)
        {
            RivalController->NotifyProvokedBy(OwnerBike);
        }

        const URIParticipantComponent* OtherParticipant = OtherBike->GetParticipantComponent();
        const bool bOtherHuman = OtherParticipant && OtherParticipant->IsHumanControlled();

        OtherBike->TriggerComicImpact(-Side, bOtherHuman ? TEXT("WHACK!") : TEXT("SMACK!"), 0.72f);
        return true;
    }

    return false;
}
