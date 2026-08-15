#include "AI/RIRivalChaosSubsystem.h"

#include "AI/RIAIController.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIParticipantComponent.h"
#include "EngineUtils.h"

namespace
{
    int32 RIChaos_GetBotIndex(const ARIBikePawn* Bike)
    {
        if (!Bike || !Bike->GetParticipantComponent()) return 0;

        FString Id = Bike->GetParticipantComponent()->GetParticipantId().ToString();
        if (!Id.RemoveFromStart(TEXT("BOT_"), ESearchCase::IgnoreCase))
        {
            return 0;
        }
        return FMath::Clamp(FCString::Atoi(*Id), 0, 99);
    }

    const TCHAR* RIChaos_GetRoleName(const int32 BotIndex)
    {
        switch (BotIndex)
        {
        case 1: return TEXT("LEECH");
        case 2: return TEXT("HOTHEAD");
        case 3: return TEXT("PETTY");
        case 4: return TEXT("GREMLIN");
        case 5: return TEXT("BRAWLER");
        case 6: return TEXT("TRYHARD");
        default: return TEXT("IDIOT");
        }
    }
}

bool URIRivalChaosSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIRivalChaosSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIRivalChaosSubsystem, STATGROUP_Tickables);
}

float URIRivalChaosSubsystem::GetDirectiveInterval(const int32 BotIndex) const
{
    switch (BotIndex)
    {
    case 1: return 9.0f;   // Leech still cares about the race most of the time.
    case 2: return 4.8f;   // Hothead actively starts trouble.
    case 3: return 6.8f;   // Petty periodically chooses someone to annoy.
    case 4: return 4.2f;   // Gremlin's actual objective is disruption.
    case 5: return 5.2f;   // Brawler hunts close-range confrontations.
    case 6: return 30.0f;  // Tryhard is intentionally race-focused.
    default: return 8.0f;
    }
}

ARIBikePawn* URIRivalChaosSubsystem::FindTargetFor(
    ARIAIController* Controller,
    ARIBikePawn* ControlledBike,
    const int32 BotIndex) const
{
    if (!Controller || !ControlledBike || !GetWorld() || BotIndex == 6)
    {
        return nullptr;
    }

    const FVector Origin = ControlledBike->GetActorLocation();
    const FVector Forward = ControlledBike->GetActorForwardVector().GetSafeNormal2D();
    const FVector Right = ControlledBike->GetActorRightVector().GetSafeNormal2D();

    const float MaxRange = BotIndex == 4 ? 3600.0f : (BotIndex == 2 ? 3200.0f : 2800.0f);
    ARIBikePawn* BestTarget = nullptr;
    float BestScore = TNumericLimits<float>::Max();

    for (TActorIterator<ARIBikePawn> It(GetWorld()); It; ++It)
    {
        ARIBikePawn* Candidate = *It;
        if (!Candidate || Candidate == ControlledBike || !Candidate->AreRaceControlsEnabled()) continue;

        FVector ToCandidate = Candidate->GetActorLocation() - Origin;
        ToCandidate.Z = 0.0f;
        const float Distance = ToCandidate.Size();
        if (Distance < 160.0f || Distance > MaxRange) continue;

        const FVector Direction = ToCandidate / Distance;
        const float ForwardDot = FVector::DotProduct(Direction, Forward);
        const float SideDistance = FMath::Abs(FVector::DotProduct(ToCandidate, Right));
        const bool bCandidateAI = Cast<ARIAIController>(Candidate->GetController()) != nullptr;

        float Score = Distance;

        switch (BotIndex)
        {
        case 1: // LEECH: stalk somebody mostly ahead, with a mild AI-vs-AI bias.
            if (ForwardDot < 0.05f) Score += 850.0f;
            Score *= bCandidateAI ? 0.78f : 0.96f;
            break;

        case 2: // HOTHEAD: nearest trouble wins; strongly encourage bot-on-bot fights.
            Score *= bCandidateAI ? 0.52f : 0.88f;
            if (SideDistance < 360.0f) Score *= 0.82f;
            break;

        case 3: // PETTY: likes reachable targets and does not tunnel exclusively on the player.
            Score *= bCandidateAI ? 0.68f : 0.90f;
            if (ForwardDot < -0.45f) Score += 320.0f;
            break;

        case 4: // GREMLIN: sabotage is more important than position.
            Score *= bCandidateAI ? 0.42f : 0.76f;
            Score += FMath::Abs(SideDistance - 210.0f) * 0.22f;
            break;

        case 5: // BRAWLER: seeks nearby side-by-side contact.
            Score *= bCandidateAI ? 0.58f : 0.86f;
            Score += FMath::Abs(SideDistance - 150.0f) * 0.30f;
            if (FMath::Abs(ForwardDot) < 0.65f) Score *= 0.82f;
            break;

        default:
            Score *= bCandidateAI ? 0.72f : 0.95f;
            break;
        }

        if (Score < BestScore)
        {
            BestScore = Score;
            BestTarget = Candidate;
        }
    }

    return BestTarget;
}

void URIRivalChaosSubsystem::IssueDirectives()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const double Now = World->GetTimeSeconds();

    for (TActorIterator<ARIAIController> It(World); It; ++It)
    {
        ARIAIController* Controller = *It;
        ARIBikePawn* ControlledBike = Controller ? Cast<ARIBikePawn>(Controller->GetPawn()) : nullptr;
        if (!Controller || !ControlledBike || !ControlledBike->AreRaceControlsEnabled()) continue;

        const int32 BotIndex = RIChaos_GetBotIndex(ControlledBike);
        if (BotIndex <= 0) continue;

        Controller->SetDirectorRoleLabel(RIChaos_GetRoleName(BotIndex));

        // Existing retaliation is already purposeful. Do not overwrite an active
        // grudge every few seconds or bots would jitter between objectives.
        if (Controller->GetGrudgeTimeRemaining() > 0.25f) continue;

        const TWeakObjectPtr<ARIAIController> Key(Controller);
        const double LastTime = LastDirectiveTime.FindRef(Key);
        if (LastTime > 0.0 && Now - LastTime < GetDirectiveInterval(BotIndex)) continue;

        // TRYHARD intentionally remains the race-first personality. Other roles
        // occasionally abandon the perfect racing line to create stories/chaos.
        ARIBikePawn* Target = FindTargetFor(Controller, ControlledBike, BotIndex);
        LastDirectiveTime.Add(Key, Now);
        if (!Target) continue;

        Controller->NotifyProvokedBy(Target);

        const FString SelfId = ControlledBike->GetParticipantComponent()
            ? ControlledBike->GetParticipantComponent()->GetParticipantId().ToString()
            : TEXT("BOT");
        const FString TargetId = Target->GetParticipantComponent()
            ? Target->GetParticipantComponent()->GetParticipantId().ToString()
            : TEXT("RIVAL");

        UE_LOG(LogTemp, Display, TEXT("RoadsideIdiots chaos: %s [%s] chose %s as sabotage target"),
            *SelfId,
            RIChaos_GetRoleName(BotIndex),
            *TargetId);
    }
}

void URIRivalChaosSubsystem::Tick(const float DeltaTime)
{
    DecisionRemaining -= DeltaTime;
    if (DecisionRemaining > 0.0f) return;

    DecisionRemaining = 1.25f;
    IssueDirectives();
}
