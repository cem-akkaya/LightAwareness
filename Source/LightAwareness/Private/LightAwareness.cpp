// Light Awareness System. Cem Akkaya https://www.cemakkaya.com

#include "LightAwareness.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"
#define LOCTEXT_NAMESPACE "FLightAwarenessModule"

void FLightAwarenessModule::StartupModule()
{
	// Map /Plugin/LightAwareness → <YourPlugin>/Shaders at startup
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("LightAwareness"));
	if (Plugin.IsValid())
	{
		const FString ShaderDir = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/LightAwareness"), ShaderDir);
		UE_LOG(LogTemp, Log, TEXT("[LightAwareness] Shader dir mapped: /Plugin/LightAwareness -> %s"), *ShaderDir);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LightAwareness] Plugin not found for shader mapping"));
	}
}


void FLightAwarenessModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLightAwarenessModule, LightAwareness)