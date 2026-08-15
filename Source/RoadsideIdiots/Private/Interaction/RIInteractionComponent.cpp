#include "Interaction/RIInteractionComponent.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "AI/RIAIController.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

URIInteractionComponent::URIInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool URIInteractionComponent::TrySideInteraction(float Side)
{
    ARIBikePawn* OwnerBike = Cast<ARIBikePawn>(GetOwner());
    if (!OwnerBike || !GetWorld())
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
        if (!OtherBike || OtherBike == OwnerBike)
        {
            continue;
        }

        if (UStaticMeshComponent* OtherChassis = OtherBike->GetChassis())
        {
            // The slap should read as a wobble, not just invisible sideways drift.
            OtherChassis->AddImpulse(SideDirection * SideVelocityChange + FVector::UpVector * 38.0f, NAME_None, true);

            const FVector RollKick = OtherBike->GetActorForwardVector() * (-Side * 3.6f);
            const FVector YawKick = FVector::UpVector * (Side * 0.9f);
            OtherChassis->AddAngularImpulseInRadians(RollKick + YawKick, NAME_None, true);
        }

        if (URIHealthComponent* OtherHealth = OtherBike->GetHealthComponent())
        {
            OtherHealth->ApplyImpact(ImpactCost);
        }

        RIPrototypeVisuals::PlayReaction(OtherBike, -Side);

        ARIAIController* RivalController = Cast<ARIAIController>(OtherBike->GetController());
        if (RivalController)
        {
            RivalController->NotifyProvokedBy(OwnerBike);
        }

        const URIParticipantComponent* OwnerParticipant = OwnerBike->GetParticipantComponent();
        const URIParticipantComponent* OtherParticipant = OtherBike->GetParticipantComponent();
        const bool bOwnerHuman = OwnerParticipant && OwnerParticipant->IsHumanControlled();
        const bool bOtherHuman = OtherParticipant && OtherParticipant->IsHumanControlled();
        const FString OtherName = OtherParticipant ? OtherParticipant->GetParticipantId().ToString() : TEXT("RIVAL");
        const FString OwnerName = OwnerParticipant ? OwnerParticipant->GetParticipantId().ToString() : TEXT("RIVAL");

        // Short-lived comic impact text is drawn near the victim. Human victims
        // also receive a small chase-camera kick from TriggerComicImpact.
        OtherBike->TriggerComicImpact(-Side, bOtherHuman ? TEXT("WHACK!") : TEXT("SMACK!"), 0.72f);

        if (GEngine)
        {
            if (bOwnerHuman)
            {
                const FString Personality = RivalController ? RivalController->GetPersonalityLabel() : TEXT("IDIOT");
                GEngine->AddOnScreenDebugMessage(
                    -1,
                    1.55f,
                    FColor::Yellow,
                    FString::Printf(TEXT("SMACK! %s [%s] IS MAD!"), *OtherName, *Personality));
            }
            else if (bOtherHuman)
            {
                ARIAIController* AttackerAI = Cast<ARIAIController>(OwnerBike->GetController());
                const FString Personality = AttackerAI ? AttackerAI->GetPersonalityLabel() : TEXT("IDIOT");
                GEngine->AddOnScreenDebugMessage(
                    -1,
                    1.55f,
                    FColor::Red,
                    FString::Printf(TEXT("WHACK! %s [%s] hit YOU!"), *OwnerName, *Personality));
            }
        }

        return true;
    }

    return false;
}
