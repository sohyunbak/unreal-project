// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ButterflySplineFollowComponent.generated.h"

class USplineComponent;

/** 나비 스플라인 추종 상태 (BPC_butterfly1_Child / BP_SplineFollower 연동용) */
UENUM(BlueprintType)
enum class EButterflySplinePhase : uint8
{
	/** 스플라인으로 직선 복귀 중 (가장 가까운 스플라인 상의 지점까지) */
	WantToShowOnSpline UMETA(DisplayName = "보여지고자 하는 (스플라인 복귀)"),
	/** 스플라인을 따라 이동 중 */
	ShowingOnSpline UMETA(DisplayName = "보여지는 중"),
	/** 스플라인 시작점으로 직선 이동 중 */
	HidingToSplineStart UMETA(DisplayName = "감춰지고 있는"),
	/** 시작점 도착 후 히든 */
	Hidden UMETA(DisplayName = "감춰진"),
};

/**
 * 보여짐/감춤 요청에 따라 4상태 전이 + 스플라인 추종.
 * 블루프린트에서는 Spline Component 참조를 넣고, 기존에 Hide/Visible을 주던 이벤트에서 SetShowDesired만 호출하면 됩니다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HANHWA_SH_API UButterflySplineFollowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UButterflySplineFollowComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 외부에서 "나비를 보여야 함 / 숨겨야 함" 신호를 줄 때 호출 (기존 노티파이 연결 지점) */
	UFUNCTION(BlueprintCallable, Category = "Butterfly|Spline")
	void SetShowDesired(bool bShouldShow);

	UFUNCTION(BlueprintPure, Category = "Butterfly|Spline")
	bool GetShowDesired() const { return bDesiredVisible; }

	UFUNCTION(BlueprintPure, Category = "Butterfly|Spline")
	EButterflySplinePhase GetPhase() const { return Phase; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Butterfly|Spline")
	TObjectPtr<USplineComponent> SplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Butterfly|Spline", meta = (ClampMin = "0"))
	float SplineFollowSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Butterfly|Spline", meta = (ClampMin = "0"))
	float TransitionFlySpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Butterfly|Spline", meta = (ClampMin = "0"))
	float ArrivalTolerance = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Butterfly|Spline")
	bool bFaceMovementDirection = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Butterfly|Spline")
	bool bSnapToSplineOnBeginPlay = true;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Butterfly|Spline")
	EButterflySplinePhase Phase = EButterflySplinePhase::ShowingOnSpline;

	UPROPERTY(Transient)
	bool bDesiredVisible = true;

	float DistanceAlongSpline = 0.f;

	bool AdvanceStraightTowardWorldTarget(const FVector& TargetWorld, float DeltaTime);
	float GetClosestDistanceAlongSpline(const FVector& WorldLocation) const;
	FVector GetWorldLocationAtDistanceAlongSpline(float Distance) const;
	FRotator GetWorldRotationAtDistanceAlongSpline(float Distance) const;
	void SyncDistanceAlongSplineFromActor();
	void ApplyActorHidden(bool bHidden);

	FVector GetSplineStartWorldLocation() const;
};
