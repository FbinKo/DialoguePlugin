// Copyright Epic Games, Inc. All Rights Reserved.

#include "DialoguesModule.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName DialoguesTabName("Dialogues");

#define LOCTEXT_NAMESPACE "FDialoguesModule"

void FDialoguesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FDialoguesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDialoguesModule, Dialogues)