#include "DialoguesEditorToolbar.h"
#include "DialoguesEditor.h"
#include "DialogueNode_Step.h"
#include "DialogueNode_SubNode_Requirement.h"
#include "DialoguesStyle.h"
#include "WorkflowOrientedApp/SModeWidget.h"

#define LOCTEXT_NAMESPACE "DialoguesEditorToolbar"

void FDialoguesEditorToolbar::AddModesToolbar(TSharedPtr<FExtender> Extender)
{
	check(DialoguesEditor.IsValid());
	TSharedPtr<FDialoguesEditor> DialoguesEditorPtr = DialoguesEditor.Pin();

	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		DialoguesEditorPtr->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FDialoguesEditorToolbar::FillModesToolbar));
}

void FDialoguesEditorToolbar::AddDialoguesToolbar(TSharedPtr<FExtender> Extender)
{
	check(DialoguesEditor.IsValid());
	TSharedPtr<FDialoguesEditor> DialoguesEditorPtr = DialoguesEditor.Pin();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, DialoguesEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateSP(this, &FDialoguesEditorToolbar::FillDialoguesToolbar));
	DialoguesEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FDialoguesEditorToolbar::FillModesToolbar(FToolBarBuilder& ToolbarBuilder)
{
	check(DialoguesEditor.IsValid());
	TSharedPtr<FDialoguesEditor> DialoguesEditorPtr = DialoguesEditor.Pin();

	TAttribute<FName> GetActiveMode(DialoguesEditorPtr.ToSharedRef(), &FDialoguesEditor::GetCurrentMode);
	FOnModeChangeRequested SetActiveMode = FOnModeChangeRequested::CreateSP(DialoguesEditorPtr.ToSharedRef(), &FDialoguesEditor::SetCurrentMode);

	// Left side padding
	DialoguesEditorPtr->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(4.0f, 1.0f)));

	DialoguesEditorPtr->AddToolbarWidget(
		SNew(SModeWidget, FDialoguesEditor::GetLocalizedMode(FDialoguesEditor::DialoguesMode), FDialoguesEditor::DialoguesMode)
		.OnGetActiveMode(GetActiveMode)
		.OnSetActiveMode(SetActiveMode)
		.CanBeSelected(DialoguesEditorPtr.Get(), &FDialoguesEditor::CanAccessDialoguesMode)
		.ToolTipText(LOCTEXT("DialoguesModeButtonTooltip", "Switch to Dialogues Mode"))
		.IconImage(FDialoguesStyle::Get().GetBrush("Dialogues.Graph.Icon"))
	);

	DialoguesEditorPtr->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(10.0f, 1.0f)));
}

void FDialoguesEditorToolbar::FillDialoguesToolbar(FToolBarBuilder& ToolbarBuilder)
{
	check(DialoguesEditor.IsValid());
	TSharedPtr<FDialoguesEditor> DialoguesEditorPtr = DialoguesEditor.Pin();

	if (!DialoguesEditorPtr->IsDebuggerReady() && DialoguesEditorPtr->GetCurrentMode() == FDialoguesEditor::DialoguesMode)
	{
		ToolbarBuilder.BeginSection("Dialogues");
		{
			const FText NewStepLabel = LOCTEXT("NewStep_Label", "New Step");
			const FText NewStepTooltip = LOCTEXT("NewStep_ToolTip", "Create a new task node Blueprint from a base class");
			const FSlateIcon NewStepIcon = FSlateIcon(FDialoguesStyle::Get().GetStyleSetName(), "Dialogues.Graph.Node.Step.Icon");

			ToolbarBuilder.AddComboButton(
				FUIAction(
					FExecuteAction(),
					FCanExecuteAction::CreateSP(DialoguesEditorPtr.Get(), &FDialoguesEditor::CanCreateNewClasses),
					FIsActionChecked()
				),
				FOnGetContent::CreateSP(DialoguesEditorPtr.Get(), &FDialoguesEditor::HandleCreateNewClassMenu, UDialogueNode_Step::StaticClass()),
				NewStepLabel,
				NewStepTooltip,
				NewStepIcon
			);

			const FText NewRequirementLabel = LOCTEXT("NewRequirement_Label", "New Requirement");
			const FText NewRequirementTooltip = LOCTEXT("NewRequirement_ToolTip", "Create a new task node Blueprint from a base class");
			const FSlateIcon NewRequirementIcon = FSlateIcon(FDialoguesStyle::Get().GetStyleSetName(), "Dialogues.Graph.Node.Requirement.Icon");

			ToolbarBuilder.AddComboButton(
				FUIAction(
					FExecuteAction(),
					FCanExecuteAction::CreateSP(DialoguesEditorPtr.Get(), &FDialoguesEditor::CanCreateNewClasses),
					FIsActionChecked()
				),
				FOnGetContent::CreateSP(DialoguesEditorPtr.Get(), &FDialoguesEditor::HandleCreateNewClassMenu, UDialogueNode_SubNode_Requirement::StaticClass()),
				NewRequirementLabel,
				NewRequirementTooltip,
				NewRequirementIcon
			);
		}
		ToolbarBuilder.EndSection();
	}
}

#undef LOCTEXT_NAMESPACE