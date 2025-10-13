#pragma once

#include "Framework/Commands/Commands.h"
#include "Templates/SharedPointer.h"

class FUICommandInfo;

class FDialoguesCommonCommands : public TCommands<FDialoguesCommonCommands>
{
public:
	FDialoguesCommonCommands();

	TSharedPtr<FUICommandInfo> SearchDialogues;

	/** Initialize commands */
	virtual void RegisterCommands() override;
};