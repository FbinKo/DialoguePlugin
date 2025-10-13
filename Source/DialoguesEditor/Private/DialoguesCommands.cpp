// Copyright Epic Games, Inc. All Rights Reserved.

#include "DialoguesCommands.h"

#define LOCTEXT_NAMESPACE "FDialoguesModule"

void FDialoguesCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Dialogues", "Bring up Dialogues window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
