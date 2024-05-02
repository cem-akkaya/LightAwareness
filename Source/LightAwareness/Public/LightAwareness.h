// Light Awareness System. Cem Akkaya https://www.cemakkaya.com

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FLightAwarenessModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
