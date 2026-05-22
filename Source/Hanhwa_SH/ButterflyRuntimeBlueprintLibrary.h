#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ButterflyRuntimeBlueprintLibrary.generated.h"

class UActorComponent;
class UObject;

UCLASS()
class HANHWA_SH_API UButterflyRuntimeBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Butterfly|Runtime", meta = (DisplayName = "Refresh Actor Component Runtime"))
	static bool RefreshActorComponentRuntime(UActorComponent* Component);

	UFUNCTION(BlueprintCallable, Category = "Butterfly|Runtime", meta = (DisplayName = "Rerun Owner Construction And Refresh Component"))
	static bool RerunOwnerConstructionAndRefreshComponent(UActorComponent* Component);

	UFUNCTION(BlueprintCallable, Category = "Butterfly|Runtime", meta = (DisplayName = "Compile Blueprint For Object (Editor Only)"))
	static bool CompileBlueprintForObjectEditorOnly(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Butterfly|Runtime", meta = (DisplayName = "Compile Blueprint And Refresh Component (Editor Only)"))
	static bool CompileBlueprintAndRefreshComponentEditorOnly(UActorComponent* Component);

	UFUNCTION(BlueprintCallable, Category = "Butterfly|Runtime", meta = (DisplayName = "Post Edit Change For Object (Editor Only)"))
	static bool PostEditChangeForObjectEditorOnly(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Butterfly|Runtime", meta = (DisplayName = "Post Edit Change Owner And Refresh Component (Editor Only)"))
	static bool PostEditChangeOwnerAndRefreshComponentEditorOnly(UActorComponent* Component);

	UFUNCTION(BlueprintCallable, Category = "Butterfly|Runtime", meta = (DisplayName = "Kick Butterfly Spline Follower Runtime"))
	static bool KickButterflySplineFollowerRuntime(
		UActorComponent* Component,
		bool bDesiredVisible = true,
		int32 FlyingStateValue = 1,
		FName SetupFunctionName = TEXT("SetupSpline"),
		FName DesiredVisibleVariableName = TEXT("DesiredVisible"),
		FName ButterflyStateVariableName = TEXT("ButterflyState"));
};
