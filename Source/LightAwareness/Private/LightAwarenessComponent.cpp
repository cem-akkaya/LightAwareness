// Light Awareness System. Cem Akkaya https://www.cemakkaya.com

#include "LightAwarenessComponent.h"
#include "LightAwarenessGpu.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TextureResource.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "UObject/ConstructorHelpers.h"

using namespace LightAwarenessGPU;

ULightAwarenessComponent::ULightAwarenessComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	// Component Settings
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	UActorComponent::SetAutoActivate(true);
	SetIsReplicatedByDefault(false);
	SetIsReplicated(false);
	
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

	SetupComponentPeripherals();
	CreateComponentIDTag();
}

void ULightAwarenessComponent::CreateComponentIDTag()
{
	// Define Tag
	auto CreatedTag = LightAwarenessTagPrefix + ":" + GetOwner()->GetName();

	// Add created tags
	GetOwner()->Tags.Add(FName(CreatedTag));
	this->ComponentTags.Add(FName(CreatedTag));

	// Component Add Tag Generic
	this->ComponentTags.Add(FName("LightAwarenessResponder"));
}

void ULightAwarenessComponent::SetComponentState(ELightAwarenessState State)
{
	LightAwarenessComponentState = State;

	if (LightAwarenessSubsystem)
	{
		switch (LightAwarenessComponentState)
		{
		case ELightAwarenessState::Inactive:
			LightAwarenessSubsystem->UnregisterComponent(this);
			break;
		case ELightAwarenessState::Active:
			LightAwarenessSubsystem->RegisterComponent(this);
			break;
		case ELightAwarenessState::ActiveVisible:
			LightAwarenessSubsystem->RegisterComponent(this);
			break;
		default: break; 
		}
	}
}

void ULightAwarenessComponent::CreateOwnerRenderingStateChecker()
{
	//Create a timer and delegate for rendering state checks
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, "GetRenderingState"); 
	
	OwnerMeshComponent = GetOwner()->GetComponentByClass<UMeshComponent>();

	if (OwnerMeshComponent && !OwnerMeshComponent->bHiddenInGame)
	{
		FTimerHandle TimerHandle_RenderingStateHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_RenderingStateHandle, Delegate, RenderingCheckRate, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Light Awareness : Owning Actor has no mesh component visibility, cancelling rendering state checker."));
	}
}

void ULightAwarenessComponent::GetRenderingState()
{
	if (OwnerMeshComponent->WasRecentlyRendered() && LightAwarenessComponentState != ELightAwarenessState::ActiveVisible)
	{
		SetComponentState(ELightAwarenessState::ActiveVisible);
	}
	else if ( !OwnerMeshComponent->WasRecentlyRendered() && LightAwarenessComponentState == ELightAwarenessState::ActiveVisible)
	{
		SetComponentState(ELightAwarenessState::Active);
	}
}

void ULightAwarenessComponent::SetupComponentPeripherals()
{
	// Set Detector Mesh
	SpawnRenderMesh();
		
	// Set Scene Capture
	SetupSceneCapture();

}

void ULightAwarenessComponent::BeginPlay() 
{
	Super::BeginPlay();

	// Get Subsystem
	LightAwarenessSubsystem = GetLightAwarenessSubsystem();

	// On become relevant sets component state as Active
	SetComponentState(ELightAwarenessState::Active);
	
	// Get image buffer processing area
	SetLightSensitivity(LightAwarenessSensitivity);

	// Hide possible other components interfering buffer image
	SetupOwnerOtherComponents();
	
	// If the user prefers to set up new render targets that runtime generated
	if (LightAwarenessIsRuntimeRenderTargets)
	{
		// Create A New Render Target Top
		UTextureRenderTarget2D* LARenderTargetTop = UKismetRenderingLibrary::CreateRenderTarget2D(this, 16, 16, RTF_RGBA8);
		sceneCaptureComponentTop->TextureTarget = LARenderTargetTop;

		// Create A New Render Target Top
		UTextureRenderTarget2D* LARenderTargetBottom = UKismetRenderingLibrary::CreateRenderTarget2D(this, 16, 16, RTF_RGBA8);
		sceneCaptureComponentBottom->TextureTarget = LARenderTargetBottom;

		// Update Pointers to Static RT
		LightAwarenessRenderTargetTop    = LARenderTargetTop;
		LightAwarenessRenderTargetBottom = LARenderTargetBottom;
	}
	
	// Be sure that the component visibility is set in runtime
	LightAwarenessMesh->SetVisibility(true);
	LightAwarenessMesh->SetVisibleInSceneCaptureOnly(true);
	LightAwarenessMesh->SetHiddenInSceneCapture(false);
	
	// Recalculate widths
	float LightAwarenessMeshBounds = LightAwarenessMesh->GetStaticMesh()->GetBounds().BoxExtent.Length()/2;
	sceneCaptureComponentTop->OrthoWidth = LightAwarenessMeshBounds;
	sceneCaptureComponentBottom->OrthoWidth = LightAwarenessMeshBounds;

	// Ensure Settings
	FTimerHandle SettingsTimer;
	GetWorld()->GetTimerManager().SetTimer(SettingsTimer, this, &ULightAwarenessComponent::UpdateSettings, 1.0f, false);

	// Looping Timer for a component checking its rendering state;
	CreateOwnerRenderingStateChecker();
}

void ULightAwarenessComponent::BeginDestroy()
{
	Super::BeginDestroy();

	SetComponentState(ELightAwarenessState::Inactive);
}

void ULightAwarenessComponent::SetupOwnerOtherComponents() const
{
	if (GetOwner())
	{
		TArray<UPrimitiveComponent*> AllComps;
		GetOwner()->GetComponents(AllComps);

		for (auto* Comp : AllComps)
		{
			// Don’t hide the gem
			if (Comp == LightAwarenessMesh) 
				continue;

			sceneCaptureComponentTop->HiddenComponents.Add(Comp);
			sceneCaptureComponentBottom->HiddenComponents.Add(Comp);

			UE_LOG(LogTemp, Display, TEXT("LightAwareness : Hiding %s from Scene Capture"), *Comp->GetName());
		}
	}
}

#if WITH_EDITOR
void ULightAwarenessComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UpdateSettings();
}
#endif

ULightAwarenessSubsystem* ULightAwarenessComponent::GetLightAwarenessSubsystem()
{
	if (auto Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULightAwarenessSubsystem>())
	{
		return Subsystem;
	}
	return nullptr;
}

void ULightAwarenessComponent::SpawnRenderMesh()
{
	// Create a mesh component
	LightAwarenessMesh = NewObject <UStaticMeshComponent>(this,UStaticMeshComponent::StaticClass(), TEXT("LightAwarenessDetector(Created)"));
	LightAwarenessMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	LightAwarenessMesh->SetMobility(EComponentMobility::Movable);
	LightAwarenessMesh->SetRelativeScale3D(FVector (1,1,1) * LightAwarenessDetectorScale);
	LightAwarenessMesh->SetRelativeLocation(FVector (1,1,1) * LightAwarenessDetectorOffset);
	LightAwarenessMesh->SetStaticMesh(OctahedronMesh);
	LightAwarenessMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LightAwarenessMesh->CastShadow = true;
	LightAwarenessMesh->bSelfShadowOnly = true;

	// Create and set material
	LightAwarenessMaterialDynamic = UMaterialInstanceDynamic::Create(LightAwarenessMaterial, this);
	LightAwarenessMesh->SetMaterial(0, LightAwarenessMaterialDynamic);
	
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
	// Setup Scene Capture Top
	sceneCaptureComponentTop = NewObject <USceneCaptureComponent2D>(this,USceneCaptureComponent2D::StaticClass(), TEXT("LightAwarenessSceneCaptureTop(Created)"));
	sceneCaptureComponentTop->AttachToComponent(LightAwarenessMesh, FAttachmentTransformRules::KeepRelativeTransform);

	// Setup Scene Capture Bottom
	sceneCaptureComponentBottom = NewObject <USceneCaptureComponent2D>(this,USceneCaptureComponent2D::StaticClass(), TEXT("LightAwarenessSceneCaptureBottom(Created)"));
	sceneCaptureComponentBottom->AttachToComponent(LightAwarenessMesh, FAttachmentTransformRules::KeepRelativeTransform);

	// Use constructor defined target. This is generally OK for networking too if run in separate processes
	SetupSceneCaptureSettings(sceneCaptureComponentTop, LightAwarenessRenderTargetTop, FVector (0,0,150), FRotator (-90,0,0));
	SetupSceneCaptureSettings(sceneCaptureComponentBottom, LightAwarenessRenderTargetBottom, FVector (0,0,-150) ,FRotator (90,0,0));
}

void ULightAwarenessComponent::SetupSceneCaptureSettings(USceneCaptureComponent2D* sceneCaptureComponents, UTextureRenderTarget2D* RenderTarget, const FVector& Location, const FRotator& Rotation) const
{
	// Setup Scene Capture
	sceneCaptureComponents->SetRelativeRotation(Rotation);
	sceneCaptureComponents->SetMobility(EComponentMobility::Movable);
	sceneCaptureComponents->SetHiddenInGame(false);
	sceneCaptureComponents->bCaptureOnMovement = false;
	sceneCaptureComponents->OrthoWidth = 100;
	sceneCaptureComponents->MaxViewDistanceOverride = 100;
	sceneCaptureComponents->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	sceneCaptureComponents->bUseRayTracingIfEnabled = true;
	sceneCaptureComponents->CompositeMode = SCCM_Overwrite;
	sceneCaptureComponents->ShowFlags.Lighting = true;
	sceneCaptureComponents->ShowFlags.DynamicShadows = true;
	sceneCaptureComponents->CaptureSource = SCS_FinalColorLDR;
	sceneCaptureComponents->TextureTarget = RenderTarget;
	sceneCaptureComponents->bAlwaysPersistRenderingState = true;
	sceneCaptureComponents->TextureTarget->SizeX = 16; // 16 Min Effective Shadow Catcher size for any condition
	sceneCaptureComponents->TextureTarget->SizeY = 16; // 16 Min Effective Shadow Catcher size for any condition


	// Lighting and shadows ON, everything at post-process OFF
	auto& F = sceneCaptureComponents->ShowFlags;
	F.SetLighting(true);
	F.SetMaterials(true);
	F.SetGlobalIllumination(LightAwarenessGI ? true : false);
	F.SetPostProcessing(true);
	F.SetAntiAliasing(false);
	F.SetMotionBlur(false);
	F.SetDepthOfField(false);
	F.SetLensFlares(false);
	F.SetBloom(false);
	F.SetAmbientOcclusion(false);
	F.SetScreenSpaceReflections(false);
	F.SetFog(false);
	F.SetVolumetricFog(false);
	F.SetTranslucency(false);
	F.SetSeparateTranslucency(false);
	F.SetEyeAdaptation(false);

	// Post Process Exposure
	auto& PP = sceneCaptureComponents->PostProcessSettings;
	
	PP.bOverride_AutoExposureMethod = true;
	PP.AutoExposureMethod = EAutoExposureMethod::AEM_MAX;

	// Clamp min/max to 1 so its stable
	PP.bOverride_AutoExposureMaxBrightness = false;
	PP.AutoExposureMinBrightness = 1.0f;
	
	// Enable Lumen For global illumination effects and reflections
	// Exposure is auto, however, we have setting for material intensity, rather than making exposure manual use material intensity as workaround.
	sceneCaptureComponents->PostProcessSettings.bOverride_DynamicGlobalIlluminationMethod = true;
	sceneCaptureComponents->PostProcessSettings.DynamicGlobalIlluminationMethod =
		LightAwarenessGI ? EDynamicGlobalIlluminationMethod::Lumen
						 : EDynamicGlobalIlluminationMethod::None;

	sceneCaptureComponents->bCaptureEveryFrame = false;
	sceneCaptureComponents->bConsiderUnrenderedOpaquePixelAsFullyTranslucent = true;

	// Cleanup RenderTarget
	check(RenderTarget != nullptr);
	sceneCaptureComponents->TextureTarget->RenderTargetFormat = RTF_RGBA8;
	sceneCaptureComponents->TextureTarget->ClearColor = FColor::Black;
	sceneCaptureComponents->TextureTarget->CompressionSettings = TC_VectorDisplacementmap;
	sceneCaptureComponents->TextureTarget->SRGB = 1;
	sceneCaptureComponents->DetailMode = DM_Low;
	
	// Engine Fallback
	if (LightAwarenessFallback)
	{
		// Scene Capture under 5.2 Orthographic is problematic, generally not rendering shadows at all as a known issue.
		sceneCaptureComponents->ProjectionType = ECameraProjectionMode::Perspective;
		auto CompensatedLoc = Location/2;
		sceneCaptureComponents->SetRelativeLocation(CompensatedLoc);
	}
	else
	{
		sceneCaptureComponents->ProjectionType = ECameraProjectionMode::Orthographic;
		sceneCaptureComponents->SetRelativeLocation(Location);
	}
	
#if WITH_EDITORONLY_DATA
	sceneCaptureComponents->TextureTarget->MipGenSettings = TMGS_NoMipmaps;
#endif
	sceneCaptureComponents->TextureTarget->UpdateResourceImmediate();
}

TArray<FColor> ULightAwarenessComponent::GetBufferPixels()
{
	// Clear Buffer if any
	BufferImage.Empty();
	
	// Enqueue Render Command to update texture resource depending on method
		switch (LightAwarenessMethod)
    	{
    	case ELightAwarenessDetectionMethod::Top:
    		BufferImage = RenderBufferPixelsTop();
    			break;
    	case ELightAwarenessDetectionMethod::Bottom:
    		BufferImage = RenderBufferPixelsBottom();
    			break;
		case ELightAwarenessDetectionMethod::Both:
			BufferImage = RenderBufferPixelsTop();
			BufferImage.Append(RenderBufferPixelsBottom());
			break;
    	default: break; 
    	}
	return BufferImage;
}

TArray<FColor> ULightAwarenessComponent::RenderBufferPixelsTop()
{
	TArray<FColor> PixelArray;
	sceneCaptureComponentTop->CaptureSceneDeferred();

	// Get Buffer Image Pıxel to an Array
	FTextureRenderTargetResource *RenderTargetTop = sceneCaptureComponentTop->TextureTarget->GameThread_GetRenderTargetResource();

	// Define Color Array
	TArray<FColor> CurrentBufferTop;

	// Define Search Box
	const auto SearchRectangle = FIntRect( XMin, YMin, XMax, YMax);
	RenderTargetTop->ReadPixels(CurrentBufferTop,RCM_MinMax, SearchRectangle);

	PixelArray.Append(CurrentBufferTop);
		
	return PixelArray ;
}

TArray<FColor> ULightAwarenessComponent::RenderBufferPixelsBottom()
{
	TArray<FColor> PixelArray;
	sceneCaptureComponentBottom->CaptureSceneDeferred();
	
	// Get Buffer Image Pıxel to an Array
	FTextureRenderTargetResource *RenderTargetBottom = sceneCaptureComponentBottom->TextureTarget->GameThread_GetRenderTargetResource();

	// Define Color Array
	TArray<FColor> CurrentBufferBottom;

	// Define Search Box
	const auto SearchRectangle = FIntRect( XMin, YMin, XMax, YMax);
	RenderTargetBottom->ReadPixels(CurrentBufferBottom,RCM_MinMax, SearchRectangle);

	PixelArray.Append(CurrentBufferBottom);
	return PixelArray ;
}

void ULightAwarenessComponent::ProcessLight()
{

	// Kick-starts the process for GPU or CPU.
	// GPU This is done in a separate thread to avoid blocking the game thread. CPU is also kinda async but not full.
	// Starts the processes and if CPU uses GetBufferPixels() , on GPU sends to wrapper with avg or max EnqueueConsumeAvgIfReady(), pushes to shader pass, then when readback is ready mailed back into TopMailbox/TopMailbox
	switch (LightAwarenessProcessing)
	{
	case ELightAwarenessProcessing::GPU:
		ProcessGPU();
		break;
	case ELightAwarenessProcessing::CPU:
		ProcessCPU();
		break;
	default: break; 
	}
}

void ULightAwarenessComponent::ProcessGPU()
{
	if (!GIsRHIInitialized) return;
	if (WarmupFramesRemaining > 0) { --WarmupFramesRemaining; return; }

	// Avarage
	if (LightAwarenessCalculationMethod == ELightAwarenessCalculationMethod::Average)
	{
		// schedule on render thread
		if (TopReadback.IsValid())
		{
			int32 PixelCount = sceneCaptureComponentTop && sceneCaptureComponentTop->TextureTarget
				? sceneCaptureComponentTop->TextureTarget->SizeX * sceneCaptureComponentTop->TextureTarget->SizeY
				: 0;
			EnqueueConsumeAvgIfReady(TopReadback, TopMailbox, PixelCount);
		}
		if (BottomReadback.IsValid())
		{
			int32 PixelCount = sceneCaptureComponentBottom && sceneCaptureComponentBottom->TextureTarget
				? sceneCaptureComponentBottom->TextureTarget->SizeX * sceneCaptureComponentBottom->TextureTarget->SizeY
				: 0;
			EnqueueConsumeAvgIfReady(BottomReadback, BottomMailbox, PixelCount);
		}
	}
	else // Brightest
	{
		// schedule on render thread
		if (TopReadback.IsValid())
			EnqueueConsumeMaxIfReady(TopReadback, TopMailbox);

		if (BottomReadback.IsValid())
			EnqueueConsumeMaxIfReady(BottomReadback, BottomMailbox);
	}

	// Consume 
	float LightValue = 0.f;
	if (ConsumeGpuReductions(LightValue))
	{
		if (FMath::Abs(LastLightStatusValue - LightValue) > FMath::Abs(LightUpdateStepThreshold))
		{
			OnLightAwarenessComponentUpdated.Broadcast(LightValue);
			LastLightStatusValue = LightValue;
		}
	}

	// Enqueue next frame
	KickGpuReductions();
}

void ULightAwarenessComponent::ProcessCPU()
{
	if (!GIsRHIInitialized) return;
	if (WarmupFramesRemaining > 0) { --WarmupFramesRemaining; return; }
	
	// Get Buffer Image Refresh
	GetBufferPixels();
	
	// Start an async task for buffer processing, at least unblocking
	if (BufferImage.Num() > 0)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
		{
			// Compute the light status on a background thread
			float MaxLightValue = 0.f;
			if (LightAwarenessCalculationMethod == ELightAwarenessCalculationMethod::Brightest)
			{
				for (int32 i = 0; i < BufferImage.Num(); i++)
				{
					float Luminance = FMath::Max3(BufferImage[i].R, BufferImage[i].G, BufferImage[i].B) / 255.0f;

					if (Luminance > MaxLightValue)
					{
						MaxLightValue = Luminance;
					}
				}
			}
			else
			{
				float AverageLightValue = 0.f;
	
				for (int i = 0; i < BufferImage.Num(); i++)
				{
					float Luminance = FMath::Max3(BufferImage[i].R, BufferImage[i].G, BufferImage[i].B) / 255.0f;
					AverageLightValue += Luminance;
				}
				AverageLightValue/=BufferImage.Num();
				MaxLightValue = AverageLightValue;
			}
			
			// Push the computed value to the game thread to update safely
			AsyncTask(ENamedThreads::GameThread, [this, MaxLightValue]()
			{
				if (FMath::Abs(LastLightStatusValue - MaxLightValue) > FMath::Abs(LightUpdateStepThreshold))
				{
					OnLightAwarenessComponentUpdated.Broadcast(MaxLightValue);
					LastLightStatusValue = MaxLightValue;
				}
			});
		});
	}
}

void ULightAwarenessComponent::KickGpuReductions()
{
	switch (LightAwarenessMethod)
	{
	case ELightAwarenessDetectionMethod::Top:
		if (sceneCaptureComponentTop && sceneCaptureComponentTop->TextureTarget)
		{
			if (LightAwarenessCalculationMethod == ELightAwarenessCalculationMethod::Average)
			{
				EnqueueAvgLuminanceReduce(sceneCaptureComponentTop->TextureTarget, TopReadback);
			}
			else
			{
				EnqueueMaxLuminanceReduce(sceneCaptureComponentTop->TextureTarget, TopReadback);
			}
			sceneCaptureComponentTop->CaptureSceneDeferred();
		}
		break;

	case ELightAwarenessDetectionMethod::Bottom:
		if (sceneCaptureComponentBottom && sceneCaptureComponentBottom->TextureTarget)
		{
			if (LightAwarenessCalculationMethod == ELightAwarenessCalculationMethod::Average)
			{
				EnqueueAvgLuminanceReduce(sceneCaptureComponentBottom->TextureTarget, BottomReadback);
			}
			else
			{
				EnqueueMaxLuminanceReduce(sceneCaptureComponentBottom->TextureTarget, BottomReadback);
			}
			sceneCaptureComponentBottom->CaptureSceneDeferred();
		}
		break;

	case ELightAwarenessDetectionMethod::Both:
		if (sceneCaptureComponentTop && sceneCaptureComponentTop->TextureTarget)
		{
			if (LightAwarenessCalculationMethod == ELightAwarenessCalculationMethod::Average)
			{
				EnqueueAvgLuminanceReduce(sceneCaptureComponentTop->TextureTarget, TopReadback);
			}
			else
			{
				EnqueueMaxLuminanceReduce(sceneCaptureComponentTop->TextureTarget, TopReadback);
			}
			sceneCaptureComponentTop->CaptureSceneDeferred();
		}
		if (sceneCaptureComponentBottom && sceneCaptureComponentBottom->TextureTarget)
		{
			if (LightAwarenessCalculationMethod == ELightAwarenessCalculationMethod::Average)
			{
				EnqueueAvgLuminanceReduce(sceneCaptureComponentBottom->TextureTarget, BottomReadback);
			}
			else
			{
				EnqueueMaxLuminanceReduce(sceneCaptureComponentBottom->TextureTarget, BottomReadback);
			}
			sceneCaptureComponentBottom->CaptureSceneDeferred();
		}
		break;
	}
}

// Poll readbacks
bool ULightAwarenessComponent::ConsumeGpuReductions(float& OutLightValue)
{
	bool bGotTop = false, bGotBottom = false;
	float LTop = 0.f, LBottom = 0.f;

	// mailbox check (non-blocking)
	const int32 TopEpoch = TopMailbox.Epoch.Load();
	if (TopEpoch != TopEpochSeen)
	{
		TopEpochSeen = TopEpoch;
		const uint32 Bits = TopMailbox.Bits.Load();
		LTop = *reinterpret_cast<const float*>(&Bits);
		bGotTop = true;
	}

	const int32 BottomEpoch = BottomMailbox.Epoch.Load();
	if (BottomEpoch != BottomEpochSeen)
	{
		BottomEpochSeen = BottomEpoch;
		const uint32 Bits = BottomMailbox.Bits.Load();
		LBottom = *reinterpret_cast<const float*>(&Bits);
		bGotBottom = true;
	}

	if (!bGotTop && !bGotBottom)
		return false;

	// ---- Calculation driven by method ----
	if (LightAwarenessCalculationMethod == ELightAwarenessCalculationMethod::Brightest)
	{
		float Brightest = 0.f;
		if (bGotTop)    Brightest = FMath::Max(Brightest, LTop);
		if (bGotBottom) Brightest = FMath::Max(Brightest, LBottom);
		OutLightValue = Brightest;
	}
	else // Average
	{
		float Sum   = 0.f;
		int   Count = 0;
		if (bGotTop)    { Sum += LTop;    ++Count; }
		if (bGotBottom) { Sum += LBottom; ++Count; }
		OutLightValue = (Count > 0) ? (Sum / Count) : 0.f;
	}

	return true;
}

void ULightAwarenessComponent::SetLightSensitivity(ELightAwarenessSensitivity Sensitivity)
{
	switch (Sensitivity)
	{
	case ELightAwarenessSensitivity::Optimized:
		RenderWidth = RenderHeight = 8; // 64 Pixels Buffer Array
		break;
	case ELightAwarenessSensitivity::Low:
		RenderWidth = RenderHeight = 4; // 16 Pixels Buffer Array
		break;
	case ELightAwarenessSensitivity::High:
		RenderWidth = RenderHeight = 16; // 256  Pixels Buffer Array
		break;
	default:RenderWidth = RenderHeight = 8;
		break;
	}
	
	// Get Detail and Optimize Calculation of Brute Force
	// Removing borders for more responsive detection
	YMin = 0;
	YMax = RenderHeight;
	XMin = 0;
	XMax = RenderWidth;

	// Reserve Array for memory allocation
	BufferImage.Reserve(UKismetMathLibrary::Square(RenderWidth));
	BufferImage.SetNum(UKismetMathLibrary::Square(RenderWidth));
}

void ULightAwarenessComponent::UpdateSettings() const
{
	if (LightAwarenessMesh)
	{
		LightAwarenessMesh->SetRelativeScale3D(FVector (1,1,1) * LightAwarenessDetectorScale);
		LightAwarenessMesh->SetRelativeLocation(FVector (1,1,1) * LightAwarenessDetectorOffset);
	}
	if (LightAwarenessMaterialDynamic)
	{
		LightAwarenessMaterialDynamic->SetScalarParameterValue("MatSensitivity", LightAwarenessMaterialSensitivity);
	}
	
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
	
	//Always Set rotation world to overcome value peaks regarding the method
	LightAwarenessMesh->SetWorldRotation(FRotator(0.0f, 0.0f, 0.0f));

	// Method Update On Distance Delta Threshold Reached
	if (LightAwarenessGetMethod == ELightAwarenessGetMethod::Distance)
	{
		auto CurrentOwnerLocation = GetOwner()->GetActorLocation();
		auto CurrentDistanceDelta = UKismetMathLibrary::Abs(UKismetMathLibrary::Vector_Distance2D(CurrentOwnerLocation, LastUpdateWorldPosition));
		if (CurrentDistanceDelta > abs(DistanceDeltaForUpdate))
		{
			ProcessLight();
			LastUpdateWorldPosition = CurrentOwnerLocation;
		}
	}

	// Method Update On Every Tick
	if (LightAwarenessGetMethod == ELightAwarenessGetMethod::EveryFrame)
	{
		ProcessLight();
	}

}

