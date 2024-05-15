// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "LightAwarenessComponent.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
struct FColor;
#ifdef LIGHTAWARENESS_LightAwarenessComponent_generated_h
#error "LightAwarenessComponent.generated.h already included, missing '#pragma once' in LightAwarenessComponent.h"
#endif
#define LIGHTAWARENESS_LightAwarenessComponent_generated_h

#define FID_HostProject_Plugins_LightAwareness_Source_LightAwareness_Public_LightAwarenessComponent_h_37_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execSetLightSensitivity); \
	DECLARE_FUNCTION(execGetBufferPixels); \
	DECLARE_FUNCTION(execGetLightStatus); \
	DECLARE_FUNCTION(execShowLightDetector); \
	DECLARE_FUNCTION(execHideLightDetector);


#define FID_HostProject_Plugins_LightAwareness_Source_LightAwareness_Public_LightAwarenessComponent_h_37_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesULightAwarenessComponent(); \
	friend struct Z_Construct_UClass_ULightAwarenessComponent_Statics; \
public: \
	DECLARE_CLASS(ULightAwarenessComponent, UActorComponent, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/LightAwareness"), NO_API) \
	DECLARE_SERIALIZER(ULightAwarenessComponent)


#define FID_HostProject_Plugins_LightAwareness_Source_LightAwareness_Public_LightAwarenessComponent_h_37_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	ULightAwarenessComponent(ULightAwarenessComponent&&); \
	ULightAwarenessComponent(const ULightAwarenessComponent&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, ULightAwarenessComponent); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(ULightAwarenessComponent); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(ULightAwarenessComponent) \
	NO_API virtual ~ULightAwarenessComponent();


#define FID_HostProject_Plugins_LightAwareness_Source_LightAwareness_Public_LightAwarenessComponent_h_34_PROLOG
#define FID_HostProject_Plugins_LightAwareness_Source_LightAwareness_Public_LightAwarenessComponent_h_37_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_HostProject_Plugins_LightAwareness_Source_LightAwareness_Public_LightAwarenessComponent_h_37_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_HostProject_Plugins_LightAwareness_Source_LightAwareness_Public_LightAwarenessComponent_h_37_INCLASS_NO_PURE_DECLS \
	FID_HostProject_Plugins_LightAwareness_Source_LightAwareness_Public_LightAwarenessComponent_h_37_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> LIGHTAWARENESS_API UClass* StaticClass<class ULightAwarenessComponent>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_HostProject_Plugins_LightAwareness_Source_LightAwareness_Public_LightAwarenessComponent_h


#define FOREACH_ENUM_FLIGHTAWARENESSSENSITIVITY(op) \
	op(Optimized) \
	op(Low) \
	op(High) 

enum FLightAwarenessSensitivity : int;
template<> LIGHTAWARENESS_API UEnum* StaticEnum<FLightAwarenessSensitivity>();

#define FOREACH_ENUM_FLIGHTAWARENESSDETECTIONMETHOD(op) \
	op(Single) \
	op(Double) 

enum FLightAwarenessDetectionMethod : int;
template<> LIGHTAWARENESS_API UEnum* StaticEnum<FLightAwarenessDetectionMethod>();

PRAGMA_ENABLE_DEPRECATION_WARNINGS
