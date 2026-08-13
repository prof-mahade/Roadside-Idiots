#include "Visual/RIPrototypeVisuals.h"

#include "Vehicle/RIBikePawn.h"
#include "Animation/AnimSequence.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Modules/ModuleManager.h"
#include "TimerManager.h"

namespace
{
    const FName MotorcycleComponentName(TEXT("PrototypeMotorcycleVisual"));
    const FName RiderComponentName(TEXT("PrototypeRiderVisual"));

    struct FRIAnimationPair
    {
        TObjectPtr<UAnimSequence> Rider = nullptr;
        TObjectPtr<UAnimSequence> Bike = nullptr;
    };

    IAssetRegistry& GetRegistry()
    {
        FAssetRegistryModule& Module = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
        return Module.Get();
    }

    template <typename TAsset>
    TAsset* FindAssetByName(const FName RootPath, const FName AssetName)
    {
        TArray<FAssetData> Assets;
        GetRegistry().GetAssetsByPath(RootPath, Assets, true, false);
        for (const FAssetData& Asset : Assets)
        {
            if (Asset.AssetName == AssetName)
            {
                return Cast<TAsset>(Asset.GetAsset());
            }
        }
        return nullptr;
    }

    USkeletalMeshComponent* FindVisualComponent(ARIBikePawn* Bike, const FName ComponentName)
    {
        if (!Bike) return nullptr;

        TArray<USkeletalMeshComponent*> Components;
        Bike->GetComponents<USkeletalMeshComponent>(Components);
        for (USkeletalMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == ComponentName)
            {
                return Component;
            }
        }
        return nullptr;
    }

    void GetAnimationAssets(TArray<FAssetData>& OutAssets)
    {
        OutAssets.Reset();
        GetRegistry().GetAssetsByPath(FName(TEXT("/Game/MotoInteractionAnims/Animations")), OutAssets, true, false);
    }

    UAnimSequence* FindAnimationByAssetName(const TArray<FAssetData>& Assets, const FString& AssetName)
    {
        for (const FAssetData& Asset : Assets)
        {
            if (Asset.AssetName.ToString().Equals(AssetName, ESearchCase::IgnoreCase))
            {
                return Cast<UAnimSequence>(Asset.GetAsset());
            }
        }
        return nullptr;
    }

    FRIAnimationPair FindStraightRidingPair()
    {
        TArray<FAssetData> Assets;
        GetAnimationAssets(Assets);

        FRIAnimationPair BestPair;
        int32 BestScore = TNumericLimits<int32>::Lowest();

        for (const FAssetData& Asset : Assets)
        {
            const FString PackagePath = Asset.PackagePath.ToString();
            if (!PackagePath.Contains(TEXT("Riding"), ESearchCase::IgnoreCase))
            {
                continue;
            }

            const FString Name = Asset.AssetName.ToString();
            if (Name.Contains(TEXT("_Bike"), ESearchCase::IgnoreCase))
            {
                continue;
            }

            UAnimSequence* RiderSequence = Cast<UAnimSequence>(Asset.GetAsset());
            if (!RiderSequence)
            {
                continue;
            }

            int32 Score = 0;
            if (Name.Contains(TEXT("Ride"), ESearchCase::IgnoreCase)) Score += 40;
            if (Name.Contains(TEXT("Loop"), ESearchCase::IgnoreCase)) Score += 35;
            if (Name.Contains(TEXT("Idle"), ESearchCase::IgnoreCase)) Score += 20;
            if (Name.Contains(TEXT("Straight"), ESearchCase::IgnoreCase)) Score += 30;
            if (Name.Contains(TEXT("Forward"), ESearchCase::IgnoreCase)) Score += 15;

            if (Name.Contains(TEXT("Turn"), ESearchCase::IgnoreCase)) Score -= 100;
            if (Name.Contains(TEXT("Left"), ESearchCase::IgnoreCase)) Score -= 35;
            if (Name.Contains(TEXT("Right"), ESearchCase::IgnoreCase)) Score -= 35;
            if (Name.Contains(TEXT("Start"), ESearchCase::IgnoreCase)) Score -= 70;
            if (Name.Contains(TEXT("Stop"), ESearchCase::IgnoreCase)) Score -= 70;
            if (Name.Contains(TEXT("Mount"), ESearchCase::IgnoreCase)) Score -= 90;
            if (Name.Contains(TEXT("Dismount"), ESearchCase::IgnoreCase)) Score -= 90;

            UAnimSequence* BikeSequence = FindAnimationByAssetName(Assets, Name + TEXT("_Bike"));
            if (BikeSequence)
            {
                Score += 80;
            }

            if (Score > BestScore)
            {
                BestScore = Score;
                BestPair.Rider = RiderSequence;
                BestPair.Bike = BikeSequence;
            }
        }

        return BestPair;
    }

    UAnimSequence* FindBestActionAnimation(const FString& FolderMarker, const float Side)
    {
        TArray<FAssetData> Assets;
        GetAnimationAssets(Assets);

        UAnimSequence* BestAnimation = nullptr;
        int32 BestScore = TNumericLimits<int32>::Lowest();

        for (const FAssetData& Asset : Assets)
        {
            const FString PackagePath = Asset.PackagePath.ToString();
            if (!PackagePath.Contains(FolderMarker, ESearchCase::IgnoreCase))
            {
                continue;
            }

            const FString Name = Asset.AssetName.ToString();
            if (Name.Contains(TEXT("_Bike"), ESearchCase::IgnoreCase))
            {
                continue;
            }

            UAnimSequence* Sequence = Cast<UAnimSequence>(Asset.GetAsset());
            if (!Sequence)
            {
                continue;
            }

            int32 Score = 0;
            if (Name.Contains(TEXT("Punch"), ESearchCase::IgnoreCase)) Score += 25;
            if (Name.Contains(TEXT("Hit"), ESearchCase::IgnoreCase)) Score += 25;
            if (Side < -0.1f && Name.Contains(TEXT("Left"), ESearchCase::IgnoreCase)) Score += 50;
            if (Side > 0.1f && Name.Contains(TEXT("Right"), ESearchCase::IgnoreCase)) Score += 50;
            if (Side < -0.1f && Name.Contains(TEXT("Right"), ESearchCase::IgnoreCase)) Score -= 30;
            if (Side > 0.1f && Name.Contains(TEXT("Left"), ESearchCase::IgnoreCase)) Score -= 30;
            if (Name.Contains(TEXT("Loop"), ESearchCase::IgnoreCase)) Score -= 20;

            if (Score > BestScore)
            {
                BestScore = Score;
                BestAnimation = Sequence;
            }
        }

        return BestAnimation;
    }

    void ResumeRidingPair(ARIBikePawn* Bike)
    {
        if (!Bike) return;

        USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
        USkeletalMeshComponent* Motorcycle = FindVisualComponent(Bike, MotorcycleComponentName);
        if (!Rider || !Motorcycle) return;

        const FRIAnimationPair Pair = FindStraightRidingPair();
        UE_LOG(
            LogTemp,
            Display,
            TEXT("RoadsideIdiots visuals: rider=%s bike=%s"),
            Pair.Rider ? *Pair.Rider->GetName() : TEXT("NONE"),
            Pair.Bike ? *Pair.Bike->GetName() : TEXT("NONE"));

        if (Pair.Bike)
        {
            Motorcycle->PlayAnimation(Pair.Bike, true);
        }
        if (Pair.Rider)
        {
            Rider->PlayAnimation(Pair.Rider, true);
        }
    }

    void ResumeLater(ARIBikePawn* Bike, UAnimSequence* Sequence)
    {
        if (!Bike || !Bike->GetWorld()) return;

        const float Delay = Sequence ? FMath::Clamp(Sequence->GetPlayLength(), 0.35f, 1.25f) : 0.65f;
        const TWeakObjectPtr<ARIBikePawn> WeakBike(Bike);
        FTimerHandle TimerHandle;
        Bike->GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateLambda([WeakBike]()
            {
                if (ARIBikePawn* ValidBike = WeakBike.Get())
                {
                    ResumeRidingPair(ValidBike);
                }
            }),
            Delay,
            false);
    }
}

void RIPrototypeVisuals::Setup(ARIBikePawn* Bike)
{
    if (!Bike || !Bike->GetRootComponent()) return;

    USkeletalMesh* MotorcycleMesh = FindAssetByName<USkeletalMesh>(FName(TEXT("/Game/MotoInteractionAnims")), FName(TEXT("SM_Bike")));
    USkeletalMesh* MannyMesh = FindAssetByName<USkeletalMesh>(FName(TEXT("/Game/Characters")), FName(TEXT("SKM_Manny_Simple")));
    if (!MotorcycleMesh || !MannyMesh) return;

    USkeletalMeshComponent* Motorcycle = FindVisualComponent(Bike, MotorcycleComponentName);
    if (!Motorcycle)
    {
        Motorcycle = NewObject<USkeletalMeshComponent>(Bike, MotorcycleComponentName);
        Bike->AddInstanceComponent(Motorcycle);
        Motorcycle->SetupAttachment(Bike->GetRootComponent());
        Motorcycle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Motorcycle->SetGenerateOverlapEvents(false);
        Motorcycle->SetRelativeLocation(FVector(0.0f, 0.0f, -28.0f));
        Motorcycle->SetRelativeRotation(FRotator::ZeroRotator);
        Motorcycle->SetRelativeScale3D(FVector::OneVector);
        Motorcycle->RegisterComponent();
    }
    Motorcycle->SetSkeletalMesh(MotorcycleMesh, true);

    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Rider)
    {
        Rider = NewObject<USkeletalMeshComponent>(Bike, RiderComponentName);
        Bike->AddInstanceComponent(Rider);
        Rider->SetupAttachment(Bike->GetRootComponent());
        Rider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Rider->SetGenerateOverlapEvents(false);
        Rider->SetRelativeLocation(FVector(0.0f, 0.0f, -28.0f));
        Rider->SetRelativeRotation(FRotator::ZeroRotator);
        Rider->SetRelativeScale3D(FVector::OneVector);
        Rider->RegisterComponent();
    }
    Rider->SetSkeletalMesh(MannyMesh, true);

    TArray<UStaticMeshComponent*> GrayboxParts;
    Bike->GetComponents<UStaticMeshComponent>(GrayboxParts);
    for (UStaticMeshComponent* Part : GrayboxParts)
    {
        if (Part)
        {
            Part->SetVisibility(false, false);
        }
    }

    ResumeRidingPair(Bike);
}

void RIPrototypeVisuals::PlaySideAction(ARIBikePawn* Bike, float Side)
{
    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Rider) return;

    if (UAnimSequence* Sequence = FindBestActionAnimation(TEXT("Punch"), Side))
    {
        Rider->PlayAnimation(Sequence, false);
        ResumeLater(Bike, Sequence);
    }
}

void RIPrototypeVisuals::PlayReaction(ARIBikePawn* Bike, float Side)
{
    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Rider) return;

    if (UAnimSequence* Sequence = FindBestActionAnimation(TEXT("Get_Hits"), Side))
    {
        Rider->PlayAnimation(Sequence, false);
        ResumeLater(Bike, Sequence);
    }
}
