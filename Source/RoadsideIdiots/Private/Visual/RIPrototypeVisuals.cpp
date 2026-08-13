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
        bool bFreezeAtEnd = true;
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

    FRIAnimationPair FindNeutralRidingPair()
    {
        TArray<FAssetData> Assets;
        GetAnimationAssets(Assets);

        // This exact transition pair is present in the imported pack. Its final
        // frame is the clean riding pose, so freezing there is much safer than
        // looping a turn or transition animation and guessing from names.
        FRIAnimationPair Pair;
        Pair.Rider = FindAnimationByAssetName(Assets, TEXT("AS_Mounted_to_Ride"));
        Pair.Bike = FindAnimationByAssetName(Assets, TEXT("AS_Mounted_to_Ride_Bike"));
        Pair.bFreezeAtEnd = true;
        if (Pair.Rider && Pair.Bike)
        {
            return Pair;
        }

        // Fallback: the first frame of the inverse transition should represent
        // the same riding pose if the pack uses slightly different naming.
        Pair.Rider = FindAnimationByAssetName(Assets, TEXT("AS_Ride_to_Mounted"));
        Pair.Bike = FindAnimationByAssetName(Assets, TEXT("AS_Ride_to_Mounted_Bike"));
        Pair.bFreezeAtEnd = false;
        return Pair;
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

    void FreezeSequenceAtPose(USkeletalMeshComponent* Component, UAnimSequence* Sequence, bool bAtEnd)
    {
        if (!Component || !Sequence) return;

        Component->PlayAnimation(Sequence, false);
        const float PoseTime = bAtEnd ? FMath::Max(0.0f, Sequence->GetPlayLength() - 0.01f) : 0.0f;
        Component->SetPosition(PoseTime, false);
        Component->SetPlayRate(0.0f);
    }

    void ResumeRidingPair(ARIBikePawn* Bike)
    {
        if (!Bike) return;

        USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
        USkeletalMeshComponent* Motorcycle = FindVisualComponent(Bike, MotorcycleComponentName);
        if (!Rider || !Motorcycle) return;

        const FRIAnimationPair Pair = FindNeutralRidingPair();
        UE_LOG(
            LogTemp,
            Display,
            TEXT("RoadsideIdiots visuals: neutral rider=%s bike=%s pose=%s"),
            Pair.Rider ? *Pair.Rider->GetName() : TEXT("NONE"),
            Pair.Bike ? *Pair.Bike->GetName() : TEXT("NONE"),
            Pair.bFreezeAtEnd ? TEXT("END") : TEXT("START"));

        FreezeSequenceAtPose(Motorcycle, Pair.Bike, Pair.bFreezeAtEnd);
        FreezeSequenceAtPose(Rider, Pair.Rider, Pair.bFreezeAtEnd);
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
        Rider->SetPlayRate(1.0f);
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
        Rider->SetPlayRate(1.0f);
        Rider->PlayAnimation(Sequence, false);
        ResumeLater(Bike, Sequence);
    }
}
