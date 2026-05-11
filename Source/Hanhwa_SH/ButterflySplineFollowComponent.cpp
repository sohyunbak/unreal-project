// Fill out your copyright notice in the Description page of Project Settings.

#include "ButterflySplineFollowComponent.h"

#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"

UButterflySplineFollowComponent::UButterflySplineFollowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UButterflySplineFollowComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Phase == EButterflySplinePhase::Hidden)
	{
		ApplyActorHidden(true);
		return;
	}

	if (!SplineComponent || !GetOwner())
	{
		return;
	}

	if (Phase == EButterflySplinePhase::ShowingOnSpline)
	{
		SyncDistanceAlongSplineFromActor();

		if (bSnapToSplineOnBeginPlay)
		{
			const FVector Loc = GetWorldLocationAtDistanceAlongSpline(DistanceAlongSpline);
			const FRotator Rot = GetWorldRotationAtDistanceAlongSpline(DistanceAlongSpline);
			GetOwner()->SetActorLocation(Loc);
			GetOwner()->SetActorRotation(Rot);
		}
	}
}

void UButterflySplineFollowComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!SplineComponent || !GetOwner())
	{
		return;
	}

	switch (Phase)
	{
	case EButterflySplinePhase::Hidden:
		return;

	case EButterflySplinePhase::ShowingOnSpline:
	{
		const float Length = SplineComponent->GetSplineLength();
		DistanceAlongSpline = FMath::Clamp(DistanceAlongSpline + SplineFollowSpeed * DeltaTime, 0.f, Length);

		const FVector Loc = GetWorldLocationAtDistanceAlongSpline(DistanceAlongSpline);
		const FRotator Rot = GetWorldRotationAtDistanceAlongSpline(DistanceAlongSpline);
		GetOwner()->SetActorLocation(Loc);
		GetOwner()->SetActorRotation(Rot);
		break;
	}

	case EButterflySplinePhase::HidingToSplineStart:
	{
		const FVector Target = GetSplineStartWorldLocation();
		if (AdvanceStraightTowardWorldTarget(Target, DeltaTime))
		{
			Phase = EButterflySplinePhase::Hidden;
			ApplyActorHidden(true);
		}
		break;
	}

	case EButterflySplinePhase::WantToShowOnSpline:
	{
		const FVector OwnerLoc = GetOwner()->GetActorLocation();
		const float ClosestDist = GetClosestDistanceAlongSpline(OwnerLoc);
		const FVector Target = GetWorldLocationAtDistanceAlongSpline(ClosestDist);

		if (AdvanceStraightTowardWorldTarget(Target, DeltaTime))
		{
			DistanceAlongSpline = ClosestDist;
			Phase = EButterflySplinePhase::ShowingOnSpline;

			const FVector Loc = GetWorldLocationAtDistanceAlongSpline(DistanceAlongSpline);
			const FRotator Rot = GetWorldRotationAtDistanceAlongSpline(DistanceAlongSpline);
			GetOwner()->SetActorLocation(Loc);
			GetOwner()->SetActorRotation(Rot);
		}
		break;
	}

	default:
		break;
	}
}

void UButterflySplineFollowComponent::SetShowDesired(bool bShouldShow)
{
	bDesiredVisible = bShouldShow;

	if (bShouldShow)
	{
		if (Phase == EButterflySplinePhase::Hidden || Phase == EButterflySplinePhase::HidingToSplineStart)
		{
			Phase = EButterflySplinePhase::WantToShowOnSpline;
			ApplyActorHidden(false);
		}
	}
	else
	{
		if (Phase == EButterflySplinePhase::ShowingOnSpline || Phase == EButterflySplinePhase::WantToShowOnSpline)
		{
			Phase = EButterflySplinePhase::HidingToSplineStart;
		}
	}
}

bool UButterflySplineFollowComponent::AdvanceStraightTowardWorldTarget(const FVector& TargetWorld, float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return true;
	}

	FVector Loc = Owner->GetActorLocation();
	FVector ToTarget = TargetWorld - Loc;
	const float Dist = ToTarget.Size();
	if (Dist <= ArrivalTolerance)
	{
		Owner->SetActorLocation(TargetWorld);
		return true;
	}

	const FVector Dir = ToTarget / Dist;
	const float Step = TransitionFlySpeed * DeltaTime;
	const FVector NewLoc = Loc + Dir * FMath::Min(Step, Dist);
	Owner->SetActorLocation(NewLoc);

	if (bFaceMovementDirection)
	{
		Owner->SetActorRotation(Dir.Rotation());
	}

	return false;
}

float UButterflySplineFollowComponent::GetClosestDistanceAlongSpline(const FVector& WorldLocation) const
{
	if (!SplineComponent)
	{
		return 0.f;
	}

	const float InputKey = SplineComponent->FindInputKeyClosestToWorldLocation(WorldLocation);
	return SplineComponent->GetDistanceAlongSplineAtSplineInputKey(InputKey);
}

FVector UButterflySplineFollowComponent::GetWorldLocationAtDistanceAlongSpline(float Distance) const
{
	if (!SplineComponent)
	{
		return FVector::ZeroVector;
	}

	return SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FRotator UButterflySplineFollowComponent::GetWorldRotationAtDistanceAlongSpline(float Distance) const
{
	if (!SplineComponent)
	{
		return FRotator::ZeroRotator;
	}

	return SplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

void UButterflySplineFollowComponent::SyncDistanceAlongSplineFromActor()
{
	if (!GetOwner())
	{
		return;
	}

	DistanceAlongSpline = GetClosestDistanceAlongSpline(GetOwner()->GetActorLocation());
}

void UButterflySplineFollowComponent::ApplyActorHidden(bool bHidden)
{
	if (AActor* Owner = GetOwner())
	{
		Owner->SetActorHiddenInGame(bHidden);
	}
}

FVector UButterflySplineFollowComponent::GetSplineStartWorldLocation() const
{
	return GetWorldLocationAtDistanceAlongSpline(0.f);
}
