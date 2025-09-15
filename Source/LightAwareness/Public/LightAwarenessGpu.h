// Light Awareness System. Cem Akkaya https://www.cemakkaya.com


#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphResources.h"

struct FLumaMailbox
{
	TAtomic<uint32> Bits{0};
	TAtomic<int32>  Epoch{0}; 
};


// Global compute shader wrapper Max Light
class FLightMaxLumaCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FLightMaxLumaCS);
	SHADER_USE_PARAMETER_STRUCT(FLightMaxLumaCS, FGlobalShader);
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FIntPoint,                 InputSize)
		SHADER_PARAMETER(FVector3f,                 LumaWeights)
		SHADER_PARAMETER_SAMPLER(SamplerState,      PointClamp)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, InputTex)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint>, OutMax)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters&) { return true; }
};

// Global compute shader wrapper Avg Light
class FLightAvgLumaCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FLightAvgLumaCS);
	SHADER_USE_PARAMETER_STRUCT(FLightAvgLumaCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FIntPoint,                 InputSize)
		SHADER_PARAMETER(FVector3f,                 LumaWeights)
		SHADER_PARAMETER_SAMPLER(SamplerState,      PointClamp)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, InputTex)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint>, OutSum)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters&) { return true; }
};

// GPU helper API
namespace LightAwarenessGPU
{
	// Send texture to GPU and  Find Max or Average pixel value as readback
	void EnqueueMaxLuminanceReduce(UTextureRenderTarget2D* RT, TUniquePtr<FRHIGPUBufferReadback>& Readback);
	void EnqueueAvgLuminanceReduce(UTextureRenderTarget2D* RT, TUniquePtr<FRHIGPUBufferReadback>& Readback);
	
	// Que, simply checks if readback is finished then calculates max, or avg/pixelcount and sends to Mailbox as CPU to do rest
	void EnqueueConsumeMaxIfReady(TUniquePtr<FRHIGPUBufferReadback>& Readback, FLumaMailbox& Mailbox);
	void EnqueueConsumeAvgIfReady(TUniquePtr<FRHIGPUBufferReadback>& Readback, FLumaMailbox& Mailbox, int32 PixelCount);
}
