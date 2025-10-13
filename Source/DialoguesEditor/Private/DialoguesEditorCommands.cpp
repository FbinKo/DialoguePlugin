#include "DialoguesEditorCommands.h"
#include "DialoguesStyle.h"

#define LOCTEXT_NAMESPACE "DialoguesEditorCommands"

FDialoguesCommonCommands::FDialoguesCommonCommands()
	: TCommands<FDialoguesCommonCommands>("DialoguesEditor.Common", LOCTEXT("DialoguesCommandLabel", "Dialogues"), NAME_None, FDialoguesStyle::Get().GetStyleSetName())
{
}

void FDialoguesCommonCommands::RegisterCommands()
{
	UI_COMMAND(SearchDialogues, "Search", "Search these Dialogues.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::F));
}

#undef LOCTEXT_NAMESPACE