#include "Telemetry/RIRaceTelemetrySubsystem.h"

#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Core/RIRaceSettingsSubsystem.h"
#include "Race/RIRaceManager.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Vehicle/RIBikePawn.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

void URIRaceTelemetrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void URIRaceTelemetrySubsystem::Deinitialize()
{
    if (bRaceObserved && !bSummaryWritten)
    {
        WriteSummary(TEXT("world_end"));
    }

    CachedRaceManager.Reset();
    Super::Deinitialize();
}

TStatId URIRaceTelemetrySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIRaceTelemetrySubsystem, STATGROUP_Tickables);
}

void URIRaceTelemetrySubsystem::Tick(const float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || bSummaryWritten) return;

    ARIBikePawn* PlayerBike = Cast<ARIBikePawn>(UGameplayStatics::GetPlayerPawn(World, 0));
    if (!PlayerBike || !PlayerBike->GetParticipantComponent()) return;

    ARIRaceManager* RaceManager = CachedRaceManager.Get();
    if (!RaceManager)
    {
        for (TActorIterator<ARIRaceManager> It(World); It; ++It)
        {
            RaceManager = *It;
            CachedRaceManager = RaceManager;
            break;
        }
    }
    if (!RaceManager) return;

    if (!RaceManager->IsRaceStarted())
    {
        // Refresh the baseline while the grid is settling so spawn-time physics
        // and setup changes never become fake playtest events.
        LastPlace = RaceManager->GetPlace(PlayerBike->GetParticipantComponent()->GetParticipantId());
        LastBananaCount = PlayerBike->GetBananaPeelCount();
        LastEggCount = PlayerBike->GetRottenEggCount();
        if (const URIHealthComponent* Health = PlayerBike->GetHealthComponent())
        {
            LastImpactSerial = Health->GetImpactSerial();
        }
        return;
    }

    if (!bRaceObserved)
    {
        BeginObservation(PlayerBike, RaceManager);
    }

    SampleAccumulator += FMath::Max(0.0f, DeltaTime);
    if (SampleAccumulator < 0.20f) return;
    SampleAccumulator = FMath::Fmod(SampleAccumulator, 0.20f);

    SampleRace(PlayerBike, RaceManager);
}

void URIRaceTelemetrySubsystem::BeginObservation(ARIBikePawn* PlayerBike, ARIRaceManager* RaceManager)
{
    if (!PlayerBike || !RaceManager) return;

    bRaceObserved = true;
    ObservationStartedAt = RaceManager->GetRaceElapsedTime();
    LastPlace = RaceManager->GetPlace(PlayerBike->GetParticipantComponent()->GetParticipantId());
    LastBananaCount = PlayerBike->GetBananaPeelCount();
    LastEggCount = PlayerBike->GetRottenEggCount();
    if (const URIHealthComponent* Health = PlayerBike->GetHealthComponent())
    {
        LastImpactSerial = Health->GetImpactSerial();
    }

    int32 TrafficCount = 0;
    int32 ChaosLevel = 1;
    int32 SteeringFeel = 1;
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (const URIRaceSettingsSubsystem* Settings = GameInstance->GetSubsystem<URIRaceSettingsSubsystem>())
            {
                TrafficCount = Settings->GetTrafficCount();
                ChaosLevel = Settings->GetChaosLevel();
                SteeringFeel = Settings->GetSteeringFeel();
            }
        }
    }

    const TCHAR* ChaosLabel = ChaosLevel <= 0
        ? TEXT("CLEAN")
        : (ChaosLevel >= 2 ? TEXT("MAYHEM") : TEXT("BALANCED"));
    const TCHAR* SteeringLabel = SteeringFeel <= 0
        ? TEXT("CALM")
        : (SteeringFeel >= 2 ? TEXT("QUICK") : TEXT("NORMAL"));

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI PLAYTEST START participants=%d laps=%d traffic=%d chaos=%s steering=%s"),
        RaceManager->GetParticipantCount(),
        RaceManager->GetTotalLaps(),
        TrafficCount,
        ChaosLabel,
        SteeringLabel);
}

void URIRaceTelemetrySubsystem::SampleRace(ARIBikePawn* PlayerBike, ARIRaceManager* RaceManager)
{
    if (!PlayerBike || !RaceManager || !PlayerBike->GetParticipantComponent()) return;

    const FName PlayerId = PlayerBike->GetParticipantComponent()->GetParticipantId();
    const int32 Place = RaceManager->GetPlace(PlayerId);
    if (LastPlace > 0 && Place > 0 && Place != LastPlace)
    {
        if (Place < LastPlace)
        {
            Overtakes += LastPlace - Place;
        }
        else
        {
            PositionsLost += Place - LastPlace;
        }
    }
    LastPlace = Place;

    if (URIBikeMovementComponent* Movement = PlayerBike->GetBikeMovement())
    {
        const float SpeedKph = FMath::Abs(Movement->GetForwardSpeedKph());
        SpeedSampleSum += SpeedKph;
        MaxSpeedKph = FMath::Max(MaxSpeedKph, SpeedKph);
        ++SpeedSampleCount;
    }

    if (const URIHealthComponent* Health = PlayerBike->GetHealthComponent())
    {
        const uint32 ImpactSerial = Health->GetImpactSerial();
        if (ImpactSerial != LastImpactSerial)
        {
            // Impact immunity is 0.65 s while telemetry samples every 0.20 s, so
            // at most one accepted condition-loss event can occur between samples.
            ++DamageEvents;
            TotalConditionLost += Health->GetLastImpactAmount();
            RecordDamageSource(Health->GetLastImpactSource());
            LastImpactSerial = ImpactSerial;
        }
    }

    const int32 BananaCount = PlayerBike->GetBananaPeelCount();
    if (BananaCount > LastBananaCount)
    {
        BananaPickups += BananaCount - LastBananaCount;
    }
    else if (BananaCount < LastBananaCount)
    {
        PeelUses += LastBananaCount - BananaCount;
    }
    LastBananaCount = BananaCount;

    const int32 EggCount = PlayerBike->GetRottenEggCount();
    if (EggCount > LastEggCount)
    {
        EggPickups += EggCount - LastEggCount;
    }
    else if (EggCount < LastEggCount)
    {
        EggUses += LastEggCount - EggCount;
    }
    LastEggCount = EggCount;

    // Comic impact text measures what the player was actually told happened.
    // It intentionally remains separate from health-loss source telemetry because
    // some readable incidents (poop, near traffic) can be disruptive without damage.
    FString ImpactText;
    float ImpactAlpha = 0.0f;
    const bool bImpactActive = PlayerBike->GetActiveComicImpact(ImpactText, ImpactAlpha);
    if (bImpactActive)
    {
        if (!bImpactWasActive || !ImpactText.Equals(LastImpactText, ESearchCase::CaseSensitive))
        {
            RecordComicIncident(ImpactText);
        }
        bImpactWasActive = true;
        LastImpactText = ImpactText;
    }
    else
    {
        bImpactWasActive = false;
        LastImpactText.Reset();
    }

    FRIRaceProgress Progress;
    if (RaceManager->GetProgress(PlayerId, Progress) && Progress.bFinished)
    {
        WriteSummary(TEXT("player_finish"));
    }
}

void URIRaceTelemetrySubsystem::RecordDamageSource(const FName SourceTag)
{
    if (SourceTag == FName(TEXT("Traffic")))
    {
        ++TrafficDamageEvents;
    }
    else if (SourceTag == FName(TEXT("Slap")))
    {
        ++SlapDamageEvents;
    }
    else if (SourceTag == FName(TEXT("Peel")))
    {
        ++PeelDamageEvents;
    }
    else if (SourceTag == FName(TEXT("Egg")))
    {
        ++EggDamageEvents;
    }
    else if (
        SourceTag == FName(TEXT("Crash")) ||
        SourceTag == FName(TEXT("CrashTip")) ||
        SourceTag == FName(TEXT("CrashPhysics")))
    {
        ++CrashDamageEvents;
    }
    else
    {
        ++UnknownDamageEvents;
    }
}

void URIRaceTelemetrySubsystem::RecordComicIncident(const FString& ImpactText)
{
    if (ImpactText.Contains(TEXT("HONK"), ESearchCase::IgnoreCase))
    {
        ++TrafficIncidents;
    }
    else if (
        ImpactText.Contains(TEXT("WHACK"), ESearchCase::IgnoreCase) ||
        ImpactText.Contains(TEXT("SMACK"), ESearchCase::IgnoreCase))
    {
        ++SlapIncidents;
    }
    else if (
        ImpactText.Contains(TEXT("SLIP"), ESearchCase::IgnoreCase) ||
        ImpactText.Contains(TEXT("OWN GOAL"), ESearchCase::IgnoreCase))
    {
        ++PeelIncidents;
    }
    else if (ImpactText.Contains(TEXT("SPLAT"), ESearchCase::IgnoreCase))
    {
        ++EggIncidents;
    }
    else if (
        ImpactText.Contains(TEXT("DOG POOP"), ESearchCase::IgnoreCase) ||
        ImpactText.Contains(TEXT("COW PATTY"), ESearchCase::IgnoreCase) ||
        ImpactText.Contains(TEXT("SPLORCH"), ESearchCase::IgnoreCase))
    {
        ++PoopIncidents;
    }
    else if (ImpactText.Contains(TEXT("DIZZY"), ESearchCase::IgnoreCase))
    {
        ++CrashIncidents;
    }
    else
    {
        ++OtherIncidents;
    }
}

void URIRaceTelemetrySubsystem::WriteSummary(const TCHAR* Reason)
{
    if (bSummaryWritten) return;
    bSummaryWritten = true;

    ARIRaceManager* RaceManager = CachedRaceManager.Get();
    ARIBikePawn* PlayerBike = GetWorld()
        ? Cast<ARIBikePawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        : nullptr;

    int32 FinalPlace = LastPlace;
    float FinishTime = RaceManager ? RaceManager->GetRaceElapsedTime() : 0.0f;
    if (RaceManager && PlayerBike && PlayerBike->GetParticipantComponent())
    {
        const FName PlayerId = PlayerBike->GetParticipantComponent()->GetParticipantId();
        FinalPlace = RaceManager->GetPlace(PlayerId);

        FRIRaceProgress Progress;
        if (RaceManager->GetProgress(PlayerId, Progress) && Progress.bFinished)
        {
            FinishTime = Progress.FinishTime;
        }
    }

    const float AverageSpeedKph = SpeedSampleCount > 0
        ? SpeedSampleSum / static_cast<float>(SpeedSampleCount)
        : 0.0f;
    const float ObservedSeconds = FMath::Max(0.0f, FinishTime - ObservationStartedAt);
    const float IncidentsPerMinute = ObservedSeconds > 1.0f
        ? static_cast<float>(DamageEvents) * 60.0f / ObservedSeconds
        : 0.0f;

    UE_LOG(LogTemp, Display, TEXT("RI PLAYTEST SUMMARY reason=%s"), Reason ? Reason : TEXT("unknown"));
    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI PLAYTEST RESULT place=%d time=%.2fs avg_speed=%.1f max_speed=%.1f"),
        FinalPlace,
        FinishTime,
        AverageSpeedKph,
        MaxSpeedKph);
    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI PLAYTEST RACECRAFT overtakes=%d positions_lost=%d damage_events=%d condition_lost=%.1f incidents_per_min=%.2f"),
        Overtakes,
        PositionsLost,
        DamageEvents,
        TotalConditionLost,
        IncidentsPerMinute);
    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI PLAYTEST ITEMS banana_pickups=%d peel_uses=%d egg_pickups=%d egg_uses=%d"),
        BananaPickups,
        PeelUses,
        EggPickups,
        EggUses);
    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI PLAYTEST DAMAGE_SOURCES crash=%d traffic=%d slap=%d peel=%d egg=%d unknown=%d"),
        CrashDamageEvents,
        TrafficDamageEvents,
        SlapDamageEvents,
        PeelDamageEvents,
        EggDamageEvents,
        UnknownDamageEvents);
    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI PLAYTEST PRESENTATION crash=%d traffic=%d slap=%d peel=%d egg=%d poop=%d other=%d"),
        CrashIncidents,
        TrafficIncidents,
        SlapIncidents,
        PeelIncidents,
        EggIncidents,
        PoopIncidents,
        OtherIncidents);
}
