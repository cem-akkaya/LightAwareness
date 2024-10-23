// Light Awareness System. Cem Akkaya https://www.cemakkaya.com

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LightAwarenessSubsystem.generated.h"

/**
 * 
 */

class ULightAwarenessComponent;

UENUM(BlueprintType)
enum class ELightAwarenessResponderState : uint8
{
	All UMETA(DisplayName = "All Registered"),
	Active UMETA(DisplayName = "Active"),
	ActiveVisible UMETA(DisplayName = "Active Rendered"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLightAwarenessSubsystemRegisteration, bool, RegistrationStatus, AActor*, Registerar, ULightAwarenessComponent*, RegisteredComponent);

UCLASS()
class LIGHTAWARENESS_API ULightAwarenessSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	TArray<ULightAwarenessComponent*> CurrentComponentsArray;
	
	void RegisterComponent(ULightAwarenessComponent* ComponentReference);

	void UnregisterComponent(ULightAwarenessComponent* ComponentReference);

	UFUNCTION(BlueprintCallable, Category = "Light Awareness Subsystem - General")
	TArray<ULightAwarenessComponent*> GetCurrentLightAwarenessResponders(ELightAwarenessResponderState State);

	UPROPERTY(BlueprintAssignable, Category="MG Interactions Component")
	FOnLightAwarenessSubsystemRegisteration OnLightAwarenessSubsystemRegisteration;
};
