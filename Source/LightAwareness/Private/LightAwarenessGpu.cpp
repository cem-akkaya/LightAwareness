// Light Awareness System. Cem Akkaya https://www.cemakkaya.com

#include "LightAwarenessGpu.h"
#include "CoreMinimal.h"

#include "RHI.h"
#include "RHIResources.h"
#include "RHICommandList.h"
#include "PipelineStateCache.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "GlobalShader.h"
#include "RHIGPUReadback.h" 
#include "ShaderParameterUtils.h"

IMPLEMENT_GLOBAL_SHADER(FLightMaxLumaCS, "/Plugin/LightAwareness/CS_LightMax.usf", "CSLightMax", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FLightAvgLumaCS, "/Plugin/LightAwareness/CS_LightAvg.usf", "CSLightAvg", SF_Compute);

static inline bool IsReadbackInFlight(const TUniquePtr<FRHIGPUBufferReadback>& RB)
{
	return RB.IsValid() && !RB->IsReady();
}

void LightAwarenessGPU::EnqueueMaxLuminanceReduce(
	UTextureRenderTarget2D* RT,
	TUniquePtr<FRHIGPUBufferReadback>& Readback)
{
	if (!RT || !GIsRHIInitialized) return;

	// If readback hasn't finished yet, skip this frame.
	if (IsReadbackInFlight(Readback)) return;

	// Ensure a readback.
	if (!Readback.IsValid())
	{
		Readback = MakeUnique<FRHIGPUBufferReadback>(TEXT("LA_LumaReadback"));
	}
	FRHIGPUBufferReadback* ReadbackPtr = Readback.Get();

	// Get the RT as an RHI texture.
	FTextureRenderTargetResource* RTRes = RT->GameThread_GetRenderTargetResource();
	FRHITexture* RHITexture = RTRes ? RTRes->GetRenderTargetTexture() : nullptr;
	if (!RHITexture || RT->SizeX <= 0 || RT->SizeY <= 0) return;

	const int32 W = RT->SizeX;
	const int32 H = RT->SizeY;

	ENQUEUE_RENDER_COMMAND(LA_ComputeLuma)(
		[RHITexture, W, H, ReadbackPtr](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);
			
			// Import the RT into RDG
			TRefCountPtr<IPooledRenderTarget> Pooled = CreateRenderTarget(RHITexture, TEXT("LA_Capture"));
			if (!Pooled.IsValid())
			{
				GraphBuilder.Execute();
				return;
			}
			FRDGTextureRef  Src    = GraphBuilder.RegisterExternalTexture(Pooled);
			FRDGTextureSRVRef SrcSRV = GraphBuilder.CreateSRV(FRDGTextureSRVDesc(Src));

			// 1 x uint structured buffer to hold the max
			FRDGBufferRef MaxBuf = GraphBuilder.CreateBuffer(
				FRDGBufferDesc::CreateStructuredDesc(sizeof(uint32), 1),
				TEXT("LA_MaxBuf"));
			FRDGBufferUAVRef MaxUAV = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(MaxBuf));
			AddClearUAVPass(GraphBuilder, MaxUAV, 0u);

			// Parameters
			FLightMaxLumaCS::FParameters* Params = GraphBuilder.AllocParameters<FLightMaxLumaCS::FParameters>();
			Params->InputSize   = FIntPoint(W, H);
			Params->LumaWeights = FVector3f(1,1,1);
			Params->PointClamp  = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
			Params->InputTex    = SrcSRV;
			Params->OutMax      = MaxUAV;

			// Shader
			TShaderMapRef<FLightMaxLumaCS> CS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
			const FIntVector Groups((W + 7) / 8, (H + 7) / 8, 1);

			// MakePass
			FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("LA_MaxLuminance"), CS, Params, Groups);

			// Async readback
			AddEnqueueCopyPass(GraphBuilder, ReadbackPtr, MaxBuf, /*OffsetBytes=*/0);
			GraphBuilder.Execute();
		});
}

void LightAwarenessGPU::EnqueueAvgLuminanceReduce(
    UTextureRenderTarget2D* RT,
    TUniquePtr<FRHIGPUBufferReadback>& Readback)
{
    if (!RT || !GIsRHIInitialized) return;

	// If readback hasn't finished yet, skip this frame.
	if (IsReadbackInFlight(Readback)) return;
	
    if (!Readback.IsValid())
        Readback = MakeUnique<FRHIGPUBufferReadback>(TEXT("LA_AvgReadback"));

    FTextureRenderTargetResource* RTRes = RT->GameThread_GetRenderTargetResource();
    FRHITexture* RHITexture = RTRes ? RTRes->GetRenderTargetTexture() : nullptr;
    if (!RHITexture) return;

    const int32 W = RT->SizeX;
    const int32 H = RT->SizeY;

    ENQUEUE_RENDER_COMMAND(LA_ComputeAvg)(
        [RHITexture, W, H, ReadbackPtr = Readback.Get()](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);

            TRefCountPtr<IPooledRenderTarget> Pooled = CreateRenderTarget(RHITexture, TEXT("LA_Capture"));
            if (!Pooled.IsValid()) { GraphBuilder.Execute(); return; }
            FRDGTextureRef    Src    = GraphBuilder.RegisterExternalTexture(Pooled);
            FRDGTextureSRVRef SrcSRV = GraphBuilder.CreateSRV(FRDGTextureSRVDesc(Src));

            FRDGBufferRef SumBuf = GraphBuilder.CreateBuffer(
                FRDGBufferDesc::CreateStructuredDesc(sizeof(uint32), 1),
                TEXT("LA_AvgBuf"));
            FRDGBufferUAVRef SumUAV = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(SumBuf));
            AddClearUAVPass(GraphBuilder, SumUAV, 0u);

            FLightAvgLumaCS::FParameters* Params = GraphBuilder.AllocParameters<FLightAvgLumaCS::FParameters>();
            Params->InputSize   = FIntPoint(W, H);
			Params->LumaWeights = FVector3f(1,1,1);
            Params->PointClamp  = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
            Params->InputTex    = SrcSRV;
            Params->OutSum      = SumUAV;

            TShaderMapRef<FLightAvgLumaCS> CS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            const FIntVector Groups((W + 7) / 8, (H + 7) / 8, 1);
            FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("LA_AvgLuminance"), CS, Params, Groups);

            AddEnqueueCopyPass(GraphBuilder, ReadbackPtr, SumBuf, 0);
            GraphBuilder.Execute();
        });
}

void LightAwarenessGPU::EnqueueConsumeMaxIfReady(
	TUniquePtr<FRHIGPUBufferReadback>& Readback,
	FLumaMailbox& Mailbox)
{
	if (!Readback.IsValid())
		return;

	FRHIGPUBufferReadback* ReadbackPtr = Readback.Get();

	ENQUEUE_RENDER_COMMAND(LA_ReadbackDrain)(
		[ReadbackPtr, &Mailbox](FRHICommandListImmediate& RHICmdList)
		{
			if (!ReadbackPtr->IsReady())
				return;

			const uint32* BitsPtr = static_cast<const uint32*>(ReadbackPtr->Lock(sizeof(uint32)));
			const uint32  Bits    = BitsPtr ? *BitsPtr : 0u;
			ReadbackPtr->Unlock();
			
			//UE_LOG(LogTemp, Warning, TEXT("LA Drain Bits=0x%08x float=%f"), Bits, *reinterpret_cast<const float*>(&Bits));

			Mailbox.Bits.Store(Bits);
			Mailbox.Epoch.AddExchange(1);

		});
}

void LightAwarenessGPU::EnqueueConsumeAvgIfReady(TUniquePtr<FRHIGPUBufferReadback>& Readback, FLumaMailbox& Mailbox,
	int32 PixelCount)
{
	if (!Readback.IsValid()) return;

	FRHIGPUBufferReadback* ReadbackPtr = Readback.Get();
	ENQUEUE_RENDER_COMMAND(LA_ReadbackDrainAvg)(
		[ReadbackPtr, &Mailbox, PixelCount](FRHICommandListImmediate&)
		{
			if (!ReadbackPtr->IsReady()) return;

			const uint32* SumPtr = static_cast<const uint32*>(ReadbackPtr->Lock(sizeof(uint32)));
			const uint32  SumFP  = SumPtr ? *SumPtr : 0u;
			ReadbackPtr->Unlock();

			const float Scale = 65536.0f;
			float Avg = (PixelCount > 0) ? (float(SumFP) / (Scale * PixelCount)) : 0.f;

			const uint32 Bits = *reinterpret_cast<uint32*>(&Avg);
			Mailbox.Bits.Store(Bits);
			Mailbox.Epoch.AddExchange(1);
		});
}


