﻿// Light Awareness System. Cem Akkaya http://www.cemakkaya.com


#include "LightAwarenessComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TextureResource.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"


ULightAwarenessComponent::ULightAwarenessComponent()
{
	// Component Settings
	PrimaryComponentTick.bCanEverTick = true;
	UActorComponent::SetAutoActivate(true);

	// Component Add Tag
	this->ComponentTags.Add(FName("LightAwarenessResponder"));

	// Refer to default Octahedron Static Mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshReferral(TEXT("/Script/Engine.StaticMesh'/LightAwareness/Octahedron.Octahedron'"));
	OctahedronMesh = MeshReferral.Object;

	// Refer to default Light Awareness Material
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialReferral(TEXT("/Script/Engine.StaticMesh'/LightAwareness/LightAwareness_M.LightAwareness_M'"));
	LightAwarenessMaterial = MaterialReferral.Object;

	// Refer to Render Targets
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RenderTargetAssetReferralTop(TEXT("/Script/Engine.TextureRenderTarget2D'/LightAwareness/LightAwarenessTop_RT.LightAwarenessTop_RT'"));
	LightAwarenessRenderTargetTop = RenderTargetAssetReferralTop.Object;

	// Refer to Render Targets
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RenderTargetAssetReferralBottom(TEXT("/Script/Engine.TextureRenderTarget2D'/LightAwareness/LightAwarenessBottom_RT.LightAwarenessBottom_RT'"));
	LightAwarenessRenderTargetBottom = RenderTargetAssetReferralBottom.Object;
}

void ULightAwarenessComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (!bIsDetectorsSpawned)
	{
		SetupComponentPeripherals();
	}
}

void ULightAwarenessComponent::SetupComponentPeripherals()
{
	// Set Detector Mesh
	SpawnRenderMesh();
		
	// Set Scene Capture
	SetupSceneCapture();

	bIsDetectorsSpawned = true;
}

void ULightAwarenessComponent::BeginPlay() 
{
	Super::BeginPlay();

	if (!bIsDetectorsSpawned)
	{
		SetupComponentPeripherals();
	}

	// Get image buffer processing area
	SetLightSensitivity();

	// Hide possible other components interfering buffer image
	SetupOwnerOtherComponents();
	
	// Be sure that the component visibility are set in runtime
	LightAwarenessMesh->SetVisibility(true);
	LightAwarenessMesh->SetVisibleInSceneCaptureOnly(true);
	LightAwarenessMesh->SetHiddenInSceneCapture(false);
}

void ULightAwarenessComponent::SetupOwnerOtherComponents() const
{
	// Clarify other components that can interfere light detection
	TArray<UStaticMeshComponent*> AvailableOwnerStaticMeshComponents;
	GetOwner()->GetComponents<UStaticMeshComponent>(AvailableOwnerStaticMeshComponents);

	// Hide all other static mesh components
	if (!AvailableOwnerStaticMeshComponents.IsEmpty())
	{
		for (const auto AvailableOwnerComponent : AvailableOwnerStaticMeshComponents)
		{
			AvailableOwnerComponent->SetHiddenInSceneCapture(true);
		}	
	}
	
	// Clarify other components that can interfere light detection
	TArray<USkeletalMeshComponent*> AvailableOwnerSkeletalMeshComponents;
	GetOwner()->GetComponents<USkeletalMeshComponent>(AvailableOwnerSkeletalMeshComponents);

	// Hide all other skeletal mesh components
	if (!AvailableOwnerSkeletalMeshComponents.IsEmpty())
	{
		for (const auto AvailableOwnerSkeletalMeshComponent : AvailableOwnerSkeletalMeshComponents)
		{
			AvailableOwnerSkeletalMeshComponent->SetHiddenInSceneCapture(true);
		}
	}
}

#if WITH_EDITOR
void ULightAwarenessComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (bIsDetectorsSpawned)
	{
		UpdateSettings();
	}
}
#endif

void ULightAwarenessComponent::SpawnRenderMesh()
{
	// Create mesh component
	LightAwarenessMesh = NewObject <UStaticMeshComponent>(this,UStaticMeshComponent::StaticClass(), TEXT("LightAwarenessDetector(Created)"));
	LightAwarenessMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	LightAwarenessMesh->SetMobility(EComponentMobility::Movable);
	LightAwarenessMesh->SetRelativeScale3D(FVector (1,1,1) * LightAwarenessDetectorScale);
	LightAwarenessMesh->SetRelativeLocation(FVector (1,1,1) * LightAwarenessDetectorOffset);
	LightAwarenessMesh->SetStaticMesh(OctahedronMesh);
	LightAwarenessMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LightAwarenessMesh->CastShadow = false;

	// Create and set material
	LightAwarenessMaterialDynamic = UMaterialInstanceDynamic::Create(LightAwarenessMaterial, this);
	LightAwarenessMesh->SetMaterial(0, LightAwarenessMaterialDynamic);
	
	LightAwarenessMesh->RegisterComponent();
	SetRenderMeshVisibility(true);
}

void ULightAwarenessComponent::SetRenderMeshVisibility(bool Status) const
{
	LightAwarenessMesh->SetVisibility(Status);
	LightAwarenessMesh->SetVisibleInSceneCaptureOnly(!Status);
	LightAwarenessMesh->SetHiddenInSceneCapture(!Status);
}

void ULightAwarenessComponent::SetupSceneCapture()
{
	//Setup Scene Capture Top
	sceneCaptureComponentTop = NewObject <USceneCaptureComponent2D>(this,USceneCaptureComponent2D::StaticClass(), TEXT("LightAwarenessSceneCaptureTop(Created)"));
	sceneCaptureComponentTop->AttachToComponent(LightAwarenessMesh, FAttachmentTransformRules::KeepRelativeTransform);
	SetupSceneCaptureSettings(sceneCaptureComponentTop, LightAwarenessRenderTargetTop, FVector (0,0,150), FRotator (-90,0,0));

	//Setup Scene Capture Bottom
	sceneCaptureComponentBottom = NewObject <USceneCaptureComponent2D>(this,USceneCaptureComponent2D::StaticClass(), TEXT("LightAwarenessSceneCaptureBottom(Created)"));
	sceneCaptureComponentBottom->AttachToComponent(LightAwarenessMesh, FAttachmentTransformRules::KeepRelativeTransform);
	SetupSceneCaptureSettings(sceneCaptureComponentBottom, LightAwarenessRenderTargetBottom, FVector (0,0,-150) ,FRotator (90,0,0));
}

void ULightAwarenessComponent::SetupSceneCaptureSettings(USceneCaptureComponent2D* sceneCaptureComponents, UTextureRenderTarget2D* RenderTarget, const FVector& Location, const FRotator& Rotation) const
{
	//Setup Scene Capture
	sceneCaptureComponents->SetRelativeRotation(Rotation);
	sceneCaptureComponents->SetRelativeLocation(Location);
	sceneCaptureComponents->ProjectionType = ECameraProjectionMode::Orthographic;
	sceneCaptureComponents->SetMobility(EComponentMobility::Movable);
	sceneCaptureComponents->bCaptureEveryFrame = false;
	sceneCaptureComponents->SetHiddenInGame(false);
	sceneCaptureComponents->bCaptureOnMovement = false;
	sceneCaptureComponents->OrthoWidth = 100;
	sceneCaptureComponents->MaxViewDistanceOverride = 100;
	sceneCaptureComponents->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	sceneCaptureComponents->ShowOnlyComponents.Add(LightAwarenessMesh);
	sceneCaptureComponents->bUseRayTracingIfEnabled = true;
	sceneCaptureComponents->CompositeMode = SCCM_Overwrite;
	sceneCaptureComponents->ShowFlags.Lighting = false;
	sceneCaptureComponents->ShowFlags.DynamicShadows = true;
	sceneCaptureComponents->CaptureSource = SCS_FinalColorLDR;
	sceneCaptureComponents->TextureTarget = RenderTarget;
	sceneCaptureComponents->bAlwaysPersistRenderingState = true;
	sceneCaptureComponents->TextureTarget->SizeX = 16; // Min Effective Shadow Catcher size for any condition
	sceneCaptureComponents->TextureTarget->SizeY = 16; // Min Effective Shadow Catcher size for any condition

	// Enable Lumen For global illumination effects and reflections
	sceneCaptureComponents->PostProcessSettings.bOverride_DynamicGlobalIlluminationMethod = LightAwarenessGI;
	sceneCaptureComponents->PostProcessSettings.DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Lumen;
	
	// Cleanup RenderTarget
	check(RenderTarget != nullptr);
	sceneCaptureComponents->TextureTarget->RenderTargetFormat = RTF_RGBA8;
	sceneCaptureComponents->TextureTarget->ClearColor = FColor::Black;
	sceneCaptureComponents->TextureTarget->CompressionSettings = TC_VectorDisplacementmap;
	sceneCaptureComponents->TextureTarget->SRGB = 0;
	
#if WITH_EDITORONLY_DATA
	sceneCaptureComponents->TextureTarget->MipGenSettings = TMGS_NoMipmaps;
#endif
	sceneCaptureComponents->TextureTarget->UpdateResourceImmediate();
	
	sceneCaptureComponents->RegisterComponent();
}

TArray<FColor> ULightAwarenessComponent::GetBufferPixels()
{
	// Clear Buffer if any
	BufferImage.Empty();
	
	// Enqueue Render Command to update texture resource depending on method
	if (LightAwarenessMethod == Single)
	{
		sceneCaptureComponentTop->CaptureScene();

		// Get Buffer Image Pıxel to an Array
		FTextureRenderTargetResource *RenderTargetTop = sceneCaptureComponentTop->TextureTarget->GameThread_GetRenderTargetResource();

		// Define Color Array
		TArray<FColor> CurrentBufferTop;

		// Define Search Box
		const auto SearchRectangle = FIntRect( XMin, YMin, XMax, YMax);
		RenderTargetTop->ReadPixels(CurrentBufferTop,RCM_MinMax, SearchRectangle);

		BufferImage.Append(CurrentBufferTop);
		
		return BufferImage ;
	}
	else
	{
		sceneCaptureComponentTop->CaptureScene();
		sceneCaptureComponentBottom->CaptureScene();

		// Get Buffer Image Pıxel to an Array
		FTextureRenderTargetResource *RenderTargetTop = sceneCaptureComponentTop->TextureTarget->GameThread_GetRenderTargetResource();
		FTextureRenderTargetResource *RenderTargetBottom = sceneCaptureComponentBottom->TextureTarget->GameThread_GetRenderTargetResource();
		
		// Define Color Array
		TArray<FColor> CurrentBufferTop;
		TArray<FColor> CurrentBufferBottom;
		
		// Define Search Box
		const auto SearchRectangle = FIntRect( XMin, YMin, XMax, YMax);
		RenderTargetTop->ReadPixels(CurrentBufferTop,RCM_MinMax, SearchRectangle);
		RenderTargetBottom->ReadPixels(CurrentBufferBottom,RCM_MinMax, SearchRectangle);

		BufferImage.Append(CurrentBufferTop);
		BufferImage.Append(CurrentBufferBottom);
		
		return BufferImage ;;
	}
}

float ULightAwarenessComponent::GetLightStatus()
{
	// Get Buffer Image Refresh
	GetBufferPixels();

	//Get Channel Max Value as Brute Force Search
	float LightValue = 0.0;
	for (int i = 0; i < BufferImage.Num(); i++)
	{
		const float AveragePixel = ((BufferImage[i].R + BufferImage[i].G + BufferImage[i].B) / 3 ) / 255.0f ;
		if ( AveragePixel > LightValue)
		{
			LightValue = AveragePixel;
		}
	}
	return LightValue ;
}

void ULightAwarenessComponent::SetLightSensitivity()
{
	switch (LightAwarenessSensitivity)
	{
	case Optimized:
		RenderWidth = RenderHeight = 8; // 32 Pixels Buffer Array
		break;
	case Low:
		RenderWidth = RenderHeight = 4; // 8 Pixels Buffer Array
		break;
	case High:
		RenderWidth = RenderHeight = 16; // 128  Pixels Buffer Array
		break;
	default:RenderWidth = RenderHeight = 8;
		break;
	}
	
	// Get Detail and Optimize Calculation of Brute Force
	// Does not change the RT Buffer since under 16px shadows cannot be caught generally.
	// Rather than changing fidelity, change the search area
	YMin = RenderHeight/2 - RenderHeight/4;
	YMax = RenderHeight/2 + RenderHeight/4;
	XMin = RenderWidth/2 - RenderWidth/4;
	XMax = RenderWidth/2 + RenderWidth/4;
}

void ULightAwarenessComponent::UpdateSettings() const
{
	LightAwarenessMesh->SetRelativeScale3D(FVector (1,1,1) * LightAwarenessDetectorScale);
	LightAwarenessMesh->SetRelativeLocation(FVector (1,1,1) * LightAwarenessDetectorOffset);
	LightAwarenessMaterialDynamic->SetScalarParameterValue("MatSensitivity", LightAwarenessMaterialSensitivity);
	
#if WITH_EDITOR
	LightAwarenessMaterial->SetScalarParameterValueEditorOnly("MatSensitivity", LightAwarenessMaterialSensitivity);
#endif
}

void ULightAwarenessComponent::ShowLightDetector() const
{
	SetRenderMeshVisibility(true);

	TArray<USceneComponent*> ChildComponents;
	LightAwarenessMesh->GetChildrenComponents(true, ChildComponents);
	for (const auto ChildComponent : ChildComponents)
	{
		ChildComponent->SetVisibility(true);
	}
}

void ULightAwarenessComponent::HideLightDetector() const
{
	SetRenderMeshVisibility(false);

	TArray<USceneComponent*> ChildComponents;
	LightAwarenessMesh->GetChildrenComponents(true, ChildComponents);
	for (const auto ChildComponent : ChildComponents)
	{
		ChildComponent->SetVisibility(false);
	}
}

void ULightAwarenessComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

