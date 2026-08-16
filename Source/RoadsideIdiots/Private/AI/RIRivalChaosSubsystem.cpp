#include "AI/RIRivalChaosSubsystem.h"

#include "AI/RIAIController.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIParticipantComponent.h"
#include "Core/RIRaceSettingsSubsystem.h"
#include "Race/RIRaceManager.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"

namespace
{
    int32 RIChaos_GetBotIndex(const ARIBikePawn* Bike)
    {
        if (!Bike || !Bike->GetParticipantComponent()) return 0;

        FString Id = Bike->GetParticipantComponent()->GetParticipantId().ToString();
        if (!Id.RemoveFromStart(TEXT("BOT_"), ESearchCase::IgnoreCase)) return 0;
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

    struct FRIChaosTuning
    {
        float IntervalScale = 1.0f;
        float ChanceScale = 1.0f;
        int32 MaxConcurrentOverride = 0;
    };

    FRIChaosTuning RIChaos_GetTuning(UWorld* World)
    {
        FRIChaosTuning Tuning;
        if (!World) return Tuning;

        UGameInstance* GameInstance = World->GetGameInstance();
        const URIRaceSettingsSubsystem* Settings = GameInstance
            ? GameInstance->GetSubsystem<URIRaceSettingsSubsystem>()
            : nullptr;
        const int32 ChaosLevel = Settings ? Settings->GetChaosLevel() : 1;

        if (ChaosLevel <= 0)
        {
            // CLEAN still allows personalities to exist and retaliation to happen,
            // but director-created incidents are intentionally uncommon.
            Tuning.IntervalScale = 1.45f;
            Tuning.ChanceScale = 0.55f;
            Tuning.MaxConcurrentOverride = 1;
        }
        else if (ChaosLevel >= 2)
        {
            // MAYHEM increases frequency, not steering authority. We deliberately
            // keep the concurrency cap conservative so the whole pack never turns
            // back into one permanent brawl.
            Tuning.IntervalScale = 0.72f;
            Tuning.ChanceScale = 1.35f;
        }

        return Tuning;
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
    case 1: return 15.0f;
    case 2: return 11.0f;
    case 3: return 14.0f;
    case 4: return 9.5f;
    case 5: return 11.5f;
    case 6: return 999.0f; // TRYHARD only retaliates; the director leaves it racing.
    default: return 14.0f;
    }
}

float URIRivalChaosSubsystem::GetDirectiveChance(const int32 BotIndex) const
{
    switch (BotIndex)
    {
    case 1: return 0.28f;
    case 2: return 0.46f;
    case 3: return 0.34f;
    case 4: return 0.58f;
    case 5: return 0.48f;
    case 6: return 0.0f;
    default: return 0.25f;
    }
}

ERITacticalIntent URIRivalChaosSubsystem::ChooseIntent(const ARIBikePawn* ControlledBike, const int32 BotIndex) const
{
    if (!ControlledBike) return ERITacticalIntent::None;

    const bool bHasEgg = ControlledBike->GetRottenEggCount() > 0;
    const bool bHasPeel = ControlledBike->GetBananaPeelCount() > 0;

    switch (BotIndex)
    {
    case 1: // LEECH: defend useful road space against the rider trying to pass.
        return ERITacticalIntent::Block;

    case 2: // HOTHEAD: close contact first, egg if armed.
        return bHasEgg && FMath::FRand() < 0.35f ? ERITacticalIntent::EggShot : ERITacticalIntent::SidePressure;

    case 3: // PETTY: weapon-focused annoyance.
        if (bHasEgg) return ERITacticalIntent::EggShot;
        if (bHasPeel) return ERITacticalIntent::PeelTrap;
        return ERITacticalIntent::Block;

    case 4: // GREMLIN: trap people whenever possible.
        if (bHasPeel) return ERITacticalIntent::PeelTrap;
        if (bHasEgg) return ERITacticalIntent::EggShot;
        return ERITacticalIntent::Block;

    case 5: // BRAWLER: short side-pressure event.
        return ERITacticalIntent::SidePressure;

    default:
        return ERITacticalIntent::None;
    }
}

ARIBikePawn* URIRivalChaosSubsystem::FindTargetFor(
    ARIAIController* Controller,
    ARIBikePawn* ControlledBike,
    const int32 BotIndex,
    const ERITacticalIntent Intent,
    const TSet<const ARIBikePawn*>& ReservedTargets) const
{
    UWorld* World = GetWorld();
    if (!Controller || !ControlledBike || !World || BotIndex == 6 || Intent == ERITacticalIntent::None) return nullptr;

    const FVector Origin = ControlledBike->GetActorLocation();
    const FVector Forward = ControlledBike->GetActorForwardVector().GetSafeNormal2D();
    const FVector Right = ControlledBike->GetActorRightVector().GetSafeNormal2D();
    const float MaxRange = BotIndex == 4 ? 2400.0f : 2100.0f;

    ARIRaceManager* RaceManager = nullptr;
    for (TActorIterator<ARIRaceManager> It(World); It; ++It)
    {
        RaceManager = *It;
        break;
    }

    int32 SelfPlace = 0;
    if (RaceManager && ControlledBike->GetParticipantComponent())
    {
        SelfPlace = RaceManager->GetPlace(ControlledBike->GetParticipantComponent()->GetParticipantId());
    }

    const bool bDefensiveIntent = Intent == ERITacticalIntent::Block || Intent == ERITacticalIntent::PeelTrap;

    ARIBikePawn* BestTarget = nullptr;
    float BestScore = TNumericLimits<float>::Max();

    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        ARIBikePawn* Candidate = *It;
        if (!Candidate || Candidate == ControlledBike || !Candidate->AreRaceControlsEnabled()) continue;
        if (ReservedTargets.Contains(Candidate)) continue;

        FVector ToCandidate = Candidate->GetActorLocation() - Origin;
        ToCandidate.Z = 0.0f;
        const float Distance = ToCandidate.Size();
        if (Distance < 220.0f || Distance > MaxRange) continue;

        const FVector Direction = ToCandidate / Distance;
        const float ForwardDot = FVector::DotProduct(Direction, Forward);
        const float SideDistance = FMath::Abs(FVector::DotProduct(ToCandidate, Right));

        // Tactics must target somebody they can physically affect. Blocking and
        // peel traps are defensive/rearward maneuvers; egg/contact pressure works
        // against riders ahead or alongside. This eliminates many fake-looking
        // directives that could never actually complete.
        if (Intent == ERITacticalIntent::PeelTrap && ForwardDot > 0.22f) continue;
        if (Intent == ERITacticalIntent::Block && ForwardDot > 0.48f) continue;
        if (Intent == ERITacticalIntent::EggShot && ForwardDot < -0.30f) continue;
        if (Intent == ERITacticalIntent::SidePressure && ForwardDot < -0.42f) continue;

        int32 CandidatePlace = 0;
        if (RaceManager && Candidate->GetParticipantComponent())
        {
            CandidatePlace = RaceManager->GetPlace(Candidate->GetParticipantComponent()->GetParticipantId());
        }

        float Score = Distance;

        if (bDefensiveIntent)
        {
            // A defensive move should answer the rider immediately behind in the
            // standings when possible, not somebody the bot has already escaped.
            if (SelfPlace > 0 && CandidatePlace > 0)
            {
                const int32 PlaceDelta = CandidatePlace - SelfPlace;
                if (PlaceDelta == 1) Score -= 330.0f;
                else if (PlaceDelta > 1) Score -= 120.0f / static_cast<float>(PlaceDelta);
                else if (PlaceDelta < 0) Score += 360.0f + 80.0f * FMath::Abs(PlaceDelta);
            }

            Score += FMath::Max(0.0f, ForwardDot) * 260.0f;
        }
        else
        {
            // Attacking/overtaking pressure belongs primarily on the position in
            // front. This makes a bot's aggression serve its race rather than
            // turning into random grief against whoever happens to be nearby.
            if (SelfPlace > 0 && CandidatePlace > 0)
            {
                const int32 PlaceDelta = CandidatePlace - SelfPlace;
                if (PlaceDelta == -1) Score -= 340.0f;
                else if (PlaceDelta < -1) Score -= 140.0f / static_cast<float>(FMath::Abs(PlaceDelta));
                else if (PlaceDelta > 0) Score += 210.0f + 75.0f * static_cast<float>(PlaceDelta);
            }

            Score -= FMath::Max(0.0f, ForwardDot) * 150.0f;
            if (ForwardDot < -0.15f) Score += 230.0f;
        }

        if (Intent == ERITacticalIntent::SidePressure)
        {
            Score += FMath::Abs(SideDistance - 170.0f) * 0.16f;
        }
        else if (Intent == ERITacticalIntent::EggShot)
        {
            // Egg throws are most credible when a target is already broadly in
            // front rather than requiring an awkward lateral chase.
            Score += SideDistance * 0.06f;
        }
        else if (BotIndex == 4)
        {
            Score += FMath::Abs(SideDistance - 220.0f) * 0.08f;
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
    const FRIChaosTuning ChaosTuning = RIChaos_GetTuning(World);
    TArray<ARIAIController*> Controllers;
    TSet<const ARIBikePawn*> ReservedTargets;
    int32 ActiveTroublemakers = 0;

    for (TActorIterator<ARIAIController> It(World); It; ++It)
    {
        ARIAIController* Controller = *It;
        ARIBikePawn* ControlledBike = Controller ? Cast<ARIBikePawn>(Controller->GetPawn()) : nullptr;
        if (!Controller || !ControlledBike || !ControlledBike->AreRaceControlsEnabled()) continue;

        Controllers.Add(Controller);
        const int32 BotIndex = RIChaos_GetBotIndex(ControlledBike);
        Controller->SetDirectorRoleLabel(RIChaos_GetRoleName(BotIndex));

        if (Controller->IsTacticalIntentActive())
        {
            ++ActiveTroublemakers;
            if (ARIBikePawn* Target = Controller->GetTacticalTarget()) ReservedTargets.Add(Target);
        }
    }

    // With 2-4 opponents allow only one deliberate chaos event at once. With
    // larger fields allow at most two. CLEAN always caps the director at one.
    int32 MaxActiveTroublemakers = Controllers.Num() >= 5 ? 2 : 1;
    if (ChaosTuning.MaxConcurrentOverride > 0)
    {
        MaxActiveTroublemakers = FMath::Min(MaxActiveTroublemakers, ChaosTuning.MaxConcurrentOverride);
    }
    if (ActiveTroublemakers >= MaxActiveTroublemakers) return;

    for (ARIAIController* Controller : Controllers)
    {
        if (!Controller || Controller->IsTacticalIntentActive() || Controller->GetTacticalCooldownRemaining() > 0.0f) continue;
        if (Controller->GetGrudgeTimeRemaining() > 0.25f) continue;

        ARIBikePawn* ControlledBike = Cast<ARIBikePawn>(Controller->GetPawn());
        const int32 BotIndex = RIChaos_GetBotIndex(ControlledBike);
        if (BotIndex <= 0 || BotIndex == 6) continue;

        const TWeakObjectPtr<ARIAIController> Key(Controller);
        const double LastTime = LastDirectiveTime.FindRef(Key);
        const float EffectiveInterval = GetDirectiveInterval(BotIndex) * ChaosTuning.IntervalScale;
        if (LastTime > 0.0 && Now - LastTime < EffectiveInterval) continue;

        // Record the attempt even when it elects to keep racing; otherwise a
        // failed chance would be retried every director tick and become 100%.
        LastDirectiveTime.Add(Key, Now);
        const float EffectiveChance = FMath::Clamp(GetDirectiveChance(BotIndex) * ChaosTuning.ChanceScale, 0.0f, 0.85f);
        if (FMath::FRand() > EffectiveChance) continue;

        const ERITacticalIntent Intent = ChooseIntent(ControlledBike, BotIndex);
        if (Intent == ERITacticalIntent::None) continue;

        ARIBikePawn* Target = FindTargetFor(Controller, ControlledBike, BotIndex, Intent, ReservedTargets);
        if (!Target) continue;

        float Duration = 2.4f;
        switch (Intent)
        {
        case ERITacticalIntent::SidePressure: Duration = 2.8f; break;
        case ERITacticalIntent::Block: Duration = 3.4f; break;
        case ERITacticalIntent::PeelTrap: Duration = 4.0f; break;
        case ERITacticalIntent::EggShot: Duration = 3.2f; break;
        default: break;
        }

        if (!Controller->AssignTacticalIntent(Target, Intent, Duration)) continue;

        ReservedTargets.Add(Target);
        ++ActiveTroublemakers;

        const FString SelfId = ControlledBike && ControlledBike->GetParticipantComponent()
            ? ControlledBike->GetParticipantComponent()->GetParticipantId().ToString()
            : TEXT("BOT");
        const FString TargetId = Target->GetParticipantComponent()
            ? Target->GetParticipantComponent()->GetParticipantId().ToString()
            : TEXT("RIVAL");

        UE_LOG(LogTemp, Display, TEXT("RoadsideIdiots tactics: %s [%s] intent=%d target=%s"),
            *SelfId, RIChaos_GetRoleName(BotIndex), static_cast<int32>(Intent), *TargetId);

        if (ActiveTroublemakers >= MaxActiveTroublemakers) break;
    }
}

void URIRivalChaosSubsystem::Tick(const float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    bool bRaceActive = false;
    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        if (*It && It->AreRaceControlsEnabled())
        {
            bRaceActive = true;
            break;
        }
    }

    if (!bRaceActive)
    {
        ActiveRaceSeconds = 0.0f;
        DecisionRemaining = 1.0f;
        return;
    }

    ActiveRaceSeconds += DeltaTime;

    // Give the player a few seconds after GO to establish speed, road position,
    // traffic and rival context. The comedy lands better after competence has
    // been established than when the starting grid instantly becomes a brawl.
    if (ActiveRaceSeconds < 6.0f) return;

    DecisionRemaining -= DeltaTime;
    if (DecisionRemaining > 0.0f) return;

    DecisionRemaining = 1.50f;
    IssueDirectives();
}
