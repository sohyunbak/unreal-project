#include "ButterflyRuntimeBlueprintLibrary.h"

#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/UnrealType.h"

#if WITH_EDITOR
#include "Engine/Blueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#endif

bool UButterflyRuntimeBlueprintLibrary::RefreshActorComponentRuntime(UActorComponent* Component)
{
	if (!IsValid(Component))
	{
		return false;
	}

	Component->Deactivate();
	Component->SetComponentTickEnabled(false);
	Component->SetAutoActivate(true);

	if (Component->IsRegistered())
	{
		Component->ReregisterComponent();
	}
	else
	{
		Component->RegisterComponent();
	}

	Component->Activate(true);
	Component->SetComponentTickEnabled(true);

	return Component->IsRegistered() && Component->IsActive() && Component->IsComponentTickEnabled();
}

bool UButterflyRuntimeBlueprintLibrary::RerunOwnerConstructionAndRefreshComponent(UActorComponent* Component)
{
	if (!IsValid(Component))
	{
		return false;
	}

	AActor* Owner = Component->GetOwner();
	if (!IsValid(Owner))
	{
		return false;
	}

	const FName ComponentName = Component->GetFName();
	const UClass* ComponentClass = Component->GetClass();

#if WITH_EDITOR
	Owner->RerunConstructionScripts();
#else
	return RefreshActorComponentRuntime(Component);
#endif

	UActorComponent* RefreshedComponent = nullptr;
	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);

	for (UActorComponent* Candidate : Components)
	{
		if (IsValid(Candidate) && Candidate->GetFName() == ComponentName)
		{
			RefreshedComponent = Candidate;
			break;
		}
	}

	if (!RefreshedComponent && ComponentClass)
	{
		for (UActorComponent* Candidate : Components)
		{
			if (IsValid(Candidate) && Candidate->IsA(ComponentClass))
			{
				RefreshedComponent = Candidate;
				break;
			}
		}
	}

	if (!IsValid(RefreshedComponent))
	{
		return false;
	}

	return RefreshActorComponentRuntime(RefreshedComponent);
}

bool UButterflyRuntimeBlueprintLibrary::CompileBlueprintForObjectEditorOnly(UObject* Object)
{
#if WITH_EDITOR
	if (!IsValid(Object))
	{
		return false;
	}

	UBlueprint* Blueprint = UBlueprint::GetBlueprintFromClass(Object->GetClass());
	if (!Blueprint)
	{
		return false;
	}

	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	return true;
#else
	return false;
#endif
}

bool UButterflyRuntimeBlueprintLibrary::CompileBlueprintAndRefreshComponentEditorOnly(UActorComponent* Component)
{
	if (!IsValid(Component))
	{
		return false;
	}

	const bool bCompiled = CompileBlueprintForObjectEditorOnly(Component);
	const bool bRefreshed = RerunOwnerConstructionAndRefreshComponent(Component);
	return bCompiled && bRefreshed;
}

bool UButterflyRuntimeBlueprintLibrary::PostEditChangeForObjectEditorOnly(UObject* Object)
{
#if WITH_EDITOR
	if (!IsValid(Object))
	{
		return false;
	}

	Object->Modify();
	Object->PostEditChange();
	return true;
#else
	return false;
#endif
}

bool UButterflyRuntimeBlueprintLibrary::PostEditChangeOwnerAndRefreshComponentEditorOnly(UActorComponent* Component)
{
	if (!IsValid(Component))
	{
		return false;
	}

	AActor* Owner = Component->GetOwner();
	if (!IsValid(Owner))
	{
		return false;
	}

	const FName ComponentName = Component->GetFName();
	const UClass* ComponentClass = Component->GetClass();

	const bool bComponentChanged = PostEditChangeForObjectEditorOnly(Component);
	const bool bOwnerChanged = PostEditChangeForObjectEditorOnly(Owner);

	UActorComponent* RefreshedComponent = nullptr;
	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);

	for (UActorComponent* Candidate : Components)
	{
		if (IsValid(Candidate) && Candidate->GetFName() == ComponentName)
		{
			RefreshedComponent = Candidate;
			break;
		}
	}

	if (!RefreshedComponent && ComponentClass)
	{
		for (UActorComponent* Candidate : Components)
		{
			if (IsValid(Candidate) && Candidate->IsA(ComponentClass))
			{
				RefreshedComponent = Candidate;
				break;
			}
		}
	}

	const bool bRefreshed = IsValid(RefreshedComponent) && RefreshActorComponentRuntime(RefreshedComponent);
	return bComponentChanged && bOwnerChanged && bRefreshed;
}

bool UButterflyRuntimeBlueprintLibrary::KickButterflySplineFollowerRuntime(
	UActorComponent* Component,
	bool bDesiredVisible,
	int32 FlyingStateValue,
	FName SetupFunctionName,
	FName DesiredVisibleVariableName,
	FName ButterflyStateVariableName)
{
	if (!IsValid(Component))
	{
		return false;
	}

	Component->SetAutoActivate(true);
	Component->Activate(true);
	Component->SetComponentTickEnabled(true);

	if (UFunction* SetupFunction = Component->FindFunction(SetupFunctionName))
	{
		Component->ProcessEvent(SetupFunction, nullptr);
	}

	if (FBoolProperty* DesiredVisibleProperty = FindFProperty<FBoolProperty>(Component->GetClass(), DesiredVisibleVariableName))
	{
		DesiredVisibleProperty->SetPropertyValue_InContainer(Component, bDesiredVisible);
	}

	if (FIntProperty* StateProperty = FindFProperty<FIntProperty>(Component->GetClass(), ButterflyStateVariableName))
	{
		StateProperty->SetPropertyValue_InContainer(Component, FlyingStateValue);
	}
	else if (FByteProperty* ByteStateProperty = FindFProperty<FByteProperty>(Component->GetClass(), ButterflyStateVariableName))
	{
		ByteStateProperty->SetPropertyValue_InContainer(Component, static_cast<uint8>(FlyingStateValue));
	}

	if (UFunction* SetupFunction = Component->FindFunction(SetupFunctionName))
	{
		Component->ProcessEvent(SetupFunction, nullptr);
	}

	return Component->IsActive() && Component->IsComponentTickEnabled();
}
