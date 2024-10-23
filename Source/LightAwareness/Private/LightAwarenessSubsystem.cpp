// Light Awareness System. Cem Akkaya https://www.cemakkaya.com


#include "LightAwarenessSubsystem.h"

#include "LightAwarenessComponent.h"

void ULightAwarenessSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentComponentsArray.Empty();
}

void ULightAwarenessSubsystem::RegisterComponent(ULightAwarenessComponent* ComponentReference)
{
	if (CurrentComponentsArray.Find(ComponentReference) == INDEX_NONE)
	{
		CurrentComponentsArray.Add(ComponentReference);	
	}
	
	OnLightAwarenessSubsystemRegisteration.Broadcast(true, ComponentReference->GetOwner(), ComponentReference);
}

void ULightAwarenessSubsystem::UnregisterComponent(ULightAwarenessComponent* ComponentReference)
{
	if (CurrentComponentsArray.Find(ComponentReference) != INDEX_NONE)
	{
		CurrentComponentsArray.Remove(ComponentReference);
		OnLightAwarenessSubsystemRegisteration.Broadcast(false, ComponentReference->GetOwner(), ComponentReference);
	}
}

TArray<ULightAwarenessComponent*> ULightAwarenessSubsystem::GetCurrentLightAwarenessResponders(ELightAwarenessResponderState State)
{
	TArray<ULightAwarenessComponent*> CurrentArray;
	
	switch (State)
	{
	case ELightAwarenessResponderState::All:
		CurrentArray = CurrentComponentsArray;
		break;
	case ELightAwarenessResponderState::ActiveVisible:

		for (ULightAwarenessComponent* CurrentComponent : CurrentComponentsArray)
		{
			if (CurrentComponent->LightAwarenessComponentState == ELightAwarenessState::ActiveVisible)
			{
				CurrentArray.Add(CurrentComponent);
			}
		}
		break;
	default: break; 
	}
	return CurrentArray;
}
