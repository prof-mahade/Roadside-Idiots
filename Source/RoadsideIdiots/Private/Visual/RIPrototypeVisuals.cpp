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
    constexpr float VisualYawOffsetDegrees = -90.0f;
    constexpr float VisualHeightOffset = -10.0f;

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

    UAnimSequence* FindBestRidingAnimation(const TArray<FAssetData>& Assets)
    {
        UAnimSequence* BestAnimation = nullptr;
        int32 BestScore = TNumericLimits<int32>::Lowest();

        for (const FAssetData& Asset : Assets)
        {
            const FString PackagePath = Asset.PackagePath.ToString();
            if (!PackagePath.Contains(TEXT("/Riding"), ESearchCase::IgnoreCase)) continue;

            const FString Name = Asset.AssetName.ToString();
            if (Name.Contains(TEXT("_Bike"), ESearchCase::IgnoreCase)) continue;

            UAnimSequence* Sequence = Cast<UAnimSequence>(Asset.GetAsset());
            if (!Sequence) continue;

            int32 Score = 100;
            if (Name.Contains(TEXT("Loop"), ESearchCase::IgnoreCase)) Score += 100;
            if (Name.Contains(TEXT("Ride"), ESearchCase::IgnoreCase)) Score += 30;
            if (Name.Contains(TEXT("Idle"), ESearchCase::IgnoreCase)) Score += 20;
            if (Name.Contains(TEXT("Turn"), ESearchCase::IgnoreCase)) Score -= 180;
            if (Name.Contains(TEXT("Left"), ESearchCase::IgnoreCase)) Score -= 90;
            if (Name.Contains(TEXT("Right"), ESearchCase::IgnoreCase)) Score -= 90;
            if (Name.Contains(TEXT("Start"), ESearchCase::IgnoreCase)) Score -= 150;
            if (Name.Contains(TEXT("Stop"), ESearchCase::IgnoreCase)) Score -= 150;
            if (Name.Contains(TEXT("Brake"), ESearchCase::IgnoreCase)) Score -= 100;
            if (Name.Contains(TEXT("Mount"), ESearchCase::IgnoreCase)) Score -= 150;

            if (Score > BestScore)
            {
                BestScore = Score;
                BestAnimation = Sequence;
            }
        }
        return BestAnimation;
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
            if (!PackagePath.Contains(FolderMarker, ESearchCase::IgnoreCase)) continue;
            const FString Name = Asset.AssetName.ToString();
            if (Name.Contains(TEXT("_Bike"), ESearchCase::IgnoreCase)) continue;
            UAnimSequence* Sequence = Cast<UAnimSequence>(Asset.GetAsset());
            if (!Sequence) continue;

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

    void ApplyNeutralPose(ARIBikePawn* Bike)
    {
        if (!Bike) return;
        USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
        USkeletalMeshComponent* Motorcycle = FindVisualComponent(Bike, MotorcycleComponentName);
        if (!Rider || !Motorcycle) return;

        Motorcycle->SetAnimation(nullptr);
        Motorcycle->SetPosition(0.0f, false);
        Motorcycle->SetPlayRate(0.0f);

        TArray<FAssetData> Assets;
        GetAnimationAssets(Assets);
        UAnimSequence* RiderPose = FindBestRidingAnimation(Assets);

        if (RiderPose)
        {
            Rider->SetPlayRate(1.0f);
            Rider->PlayAnimation(RiderPose, true);
        }
        else
        {
            RiderPose = FindAnimationByAssetName(Assets, TEXT("AS_Bike_Start"));
            if (!RiderPose) RiderPose = FindAnimationByAssetName(Assets, TEXT("AS_Mounted_to_Ride"));
            if (RiderPose)
            {
                Rider->PlayAnimation(RiderPose, false);
                Rider->SetPosition(FMath::Max(0.0f, RiderPose->GetPlayLength() - 0.01f), false);
                Rider->SetPlayRate(0.0f);
            }
        }

        UE_LOG(LogTemp, Display, TEXT("RoadsideIdiots visuals: neutral animation=%s"), RiderPose ? *RiderPose->GetName() : TEXT("NONE"));
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
                if (ARIBikePawn* ValidBike = WeakBike.Get()) ApplyNeutralPose(ValidBike);
            }),
            Delay,
            false);
    }

    void ConfigureIndependentVisual(USkeletalMeshComponent* Component)
    {
        if (!Component) return;
        Component->SetAbsolute(true, true, true);
        Component->SetWorldScale3D(FVector::OneVector);
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
        Motorcycle->RegisterComponent();
    }
    Motorcycle->SetSkeletalMesh(MotorcycleMesh, true);
    ConfigureIndependentVisual(Motorcycle);

    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Rider)
    {
        Rider = NewObject<USkeletalMeshComponent>(Bike, RiderComponentName);
        Bike->AddInstanceComponent(Rider);
        Rider->SetupAttachment(Bike->GetRootComponent());
        Rider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Rider->SetGenerateOverlapEvents(false);
        Rider->RegisterComponent();
    }
    Rider->SetSkeletalMesh(MannyMesh, true);
    ConfigureIndependentVisual(Rider);

    TArray<UStaticMeshComponent*> GrayboxParts;
    Bike->GetComponents<UStaticMeshComponent>(GrayboxParts);
    for (UStaticMeshComponent* Part : GrayboxParts) if (Part) Part->SetVisibility(false, false);

    ApplyNeutralPose(Bike);
    Update(Bike);
}

void RIPrototypeVisuals::Update(ARIBikePawn* Bike)
{
    if (!Bike) return;
    USkeletalMeshComponent* Motorcycle = FindVisualComponent(Bike, MotorcycleComponentName);
    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Motorcycle || !Rider) return;

    const FRotator ActorRotation = Bike->GetActorRotation();
    const bool bClearlyCrashed = Bike->GetActorUpVector().Z < 0.55f;
    const FRotator VisualRotation = bClearlyCrashed
        ? FRotator(ActorRotation.Pitch, ActorRotation.Yaw + VisualYawOffsetDegrees, ActorRotation.Roll)
        : FRotator(0.0f, ActorRotation.Yaw + VisualYawOffsetDegrees, 0.0f);
    const FVector VisualLocation = Bike->GetActorLocation() + FVector(0.0f, 0.0f, VisualHeightOffset);

    Motorcycle->SetWorldLocationAndRotation(VisualLocation, VisualRotation);
    Rider->SetWorldLocationAndRotation(VisualLocation, VisualRotation);
    Motorcycle->SetWorldScale3D(FVector::OneVector);
    Rider->SetWorldScale3D(FVector::OneVector);
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
