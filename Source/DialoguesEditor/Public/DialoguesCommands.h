// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "DialoguesStyle.h"

class FDialoguesCommands : public TCommands<FDialoguesCommands>
{
public:

	FDialoguesCommands()
		: TCommands<FDialoguesCommands>(TEXT("Dialogues"), NSLOCTEXT("Contexts", "Dialogues", "Dialogues Plugin"), NAME_None, FDialoguesStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};