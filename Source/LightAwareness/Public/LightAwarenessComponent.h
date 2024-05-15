// Light Awareness System. Cem Akkaya https://www.cemakkaya.com

#pragma once

#include "CoreMinimal.h"
#include "Runtime/CoreUObject/Public/UObject/SoftObjectPtr.h"
#include "Components/ActorComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "TextureResource.h"
#include "Materials/Material.h"
#include "LightAwarenessComponent.generated.h"

UENUM()
enum FLightAwarenessSensitivity : int
{
	Optimized UMETA(DisplayName = "Optimized Sensitivity"),
	Low UMETA(DisplayName = "Low Sensitivity"),
	High UMETA(DisplayName = "High Sensitivity"),
};

UENUM()
enum FLightAwarenessDetectionMethod : int
{
	Single UMETA(DisplayName = "Top Directional"),
	Double UMETA(DisplayName = "Top & Bottom Directional"),
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LIGHTAWARENESS_API ULightAwarenessComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULightAwarenessComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light Awareness" , DisplayName="Light Detector Scale", meta = (ClampMin = "0.1", ClampMax = "10", UIMin = "0.1", UIMax = "10"))
	float LightAwarenessDetectorScale = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light Awareness" , DisplayName="Light Detector Offset", meta = (ClampMin = "-500", ClampMax = "500", UIMin = "-500", UIMax = "500"))
	FVector LightAwarenessDetectorOffset = FVector(0,0,0);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light Awareness" , DisplayName="Light Detection Sensivity")
	TEnumAsByte<FLightAwarenessSensitivity> LightAwarenessSensitivity = FLightAwarenessSensitivity::Optimized;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light Awareness" , DisplayName="Light Detection Method")
	TEnumAsByte<FLightAwarenessDetectionMethod> LightAwarenessMethod = FLightAwarenessDetectionMethod::Double;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light Awareness" , DisplayName="Detect Global Illumination")
	bool LightAwarenessGI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light Awareness" , DisplayName="Engine Version Fallback")
	bool LightAwarenessFallback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light Awareness" , DisplayName="Replicate Render Targets")
	bool LightAwarenessIsReplicatedRenderTargets;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light Awareness" , DisplayName="Light Global Threshold", meta = (ClampMin = "0", ClampMax = "1", UIMin = "0", UIMax = "1"))
	float LightAwarenessMaterialSensitivity = 1;
	
	UFUNCTION(CallInEditor, Category="Light Awareness" , DisplayName="Hide Light Detector")
	void HideLightDetector() const;
	
	UFUNCTION(CallInEditor, Category="Light Awareness" , DisplayName="Show Light Detector")
	void ShowLightDetector() const;

	UFUNCTION(BlueprintCallable, Category="Light Awareness" , DisplayName="Get Light Status")
	float GetLightStatus();

	UFUNCTION(BlueprintCallable, Category ="Light Awareness" , DisplayName="Get Light Buffer")
	TArray<FColor> GetBufferPixels();
	
	UFUNCTION(BlueprintCallable, Category ="Light Awareness" , DisplayName="Set Light Sensivity")
	void SetLightSensitivity(FLightAwarenessSensitivity Sensitivity);

protected:
	// Called when the game starts.
	virtual void BeginPlay() override;

	// Create Detection Assets
	virtual void OnComponentCreated() override;

#if WITH_EDITOR
	// Editor Preview Changes
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	// Scene Capture Mesh
	void SetupComponentPeripherals();

	// Scene Capture hides render components
	void SetupOwnerOtherComponents() const;
	
	// Scene Capture Mesh
	void SpawnRenderMesh();

	// Scene Capture Mesh Visibility
	void SetRenderMeshVisibility(bool Status) const;
	
	// Static Mesh
	UStaticMesh* OctahedronMesh;

	// Static Mesh Material
	UMaterial* LightAwarenessMaterial;

	// Dynamic Material Instance
	UMaterialInstanceDynamic* LightAwarenessMaterialDynamic;

	// Render Target Top
	UTextureRenderTarget2D* LightAwarenessRenderTargetTop;

	// Render Target Bottom
	UTextureRenderTarget2D* LightAwarenessRenderTargetBottom;
	
	// Render Target Resource
	FTextureRenderTarget2DResource* LightAwarenessRenderTargetResource;
	
	// Mesh Reference
	TSoftObjectPtr<UStaticMesh> Octahedron;
	
	// Mesh Component
	UPROPERTY(VisibleDefaultsOnly, Category="Light Awareness")
	UStaticMeshComponent* LightAwarenessMesh;

	// Scene Capture Component
	UPROPERTY(VisibleDefaultsOnly, Category="Light Awareness")
	USceneCaptureComponent2D* sceneCaptureComponentTop;

	// Scene Capture Component
	UPROPERTY(VisibleDefaultsOnly, Category="Light Awareness")
	USceneCaptureComponent2D* sceneCaptureComponentBottom;
	
	// Detector Mesh Status 
	bool bIsDetectorsSpawned;

	// Editor Settings of Detector
	void UpdateSettings() const;

	// Scene Capture Setup
	void SetupSceneCapture();

	// Define Capture Settings
	void SetupSceneCaptureSettings(USceneCaptureComponent2D* sceneCaptureComponents, UTextureRenderTarget2D* RenderTarget, const FVector& Location, const FRotator& Rotation) const;

	// Render Target Variables
	int32 RenderWidth;
	int32 RenderHeight;
	int32 YMin;
	int32 YMax;
	int32 XMin;
	int32 XMax;

	// Buffer Image
	TArray<FColor> BufferImage;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
