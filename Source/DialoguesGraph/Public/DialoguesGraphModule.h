#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FObjectPreSaveContext;
class UObject;
class UPackage;

DIALOGUESGRAPH_API DECLARE_LOG_CATEGORY_EXTERN(LogDialoguesGraph, Display, All);

class FDialoguesGraphModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void HandlePreSavePackage(UPackage* Package, FObjectPreSaveContext ObjectSaveContext);
	void HandleBeginPIE(bool bIsSimulating);
};