#include "DialoguesEditorModes.h"
#include "DialoguesEditor.h"
#include "DialoguesEditorToolbar.h"
#include "DialoguesEditorTabs.h"
#include "DialoguesEditorTabFactories.h"

FDialoguesEditorApplicationMode::FDialoguesEditorApplicationMode(TSharedPtr<class FDialoguesEditor> InDialoguesEditor)
	: FApplicationMode(FDialoguesEditor::DialoguesMode, FDialoguesEditor::GetLocalizedMode)
{
	DialoguesEditor = InDialoguesEditor;

	DialoguesEditorTabFactories.RegisterFactory(MakeShareable(new FDialoguesDetailsSummoner(InDialoguesEditor)));
	DialoguesEditorTabFactories.RegisterFactory(MakeShareable(new FDialoguesSearchSummoner(InDialoguesEditor)));
	//DialoguesEditorTabFactories.RegisterFactory(MakeShareable(new FBlackboardSummoner(InDialoguesEditor)));

	TabLayout = FTabManager::NewLayout("Standalone_Dialogues_Layout")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.7f)
					->AddTab(FDialoguesEditorTabs::GraphEditorID, ETabState::ClosedTab)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.3f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->AddTab(FDialoguesEditorTabs::GraphDetailsID, ETabState::OpenedTab)
						->AddTab(FDialoguesEditorTabs::SearchID, ETabState::ClosedTab)
					)
					/*->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.4f)
						->AddTab(FDialoguesEditorTabs::BlackboardID, ETabState::OpenedTab)
					)*/
				)
			)
		);

	InDialoguesEditor->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
	//InDialoguesEditor->GetToolbarBuilder()->AddDebuggerToolbar(ToolbarExtender);
	InDialoguesEditor->GetToolbarBuilder()->AddDialoguesToolbar(ToolbarExtender);
}

void FDialoguesEditorApplicationMode::RegisterTabFactories(TSharedPtr<class FTabManager> InTabManager)
{
	check(DialoguesEditor.IsValid());
	TSharedPtr<FDialoguesEditor> DialoguesEditorPtr = DialoguesEditor.Pin();

	DialoguesEditorPtr->RegisterToolbarTab(InTabManager.ToSharedRef());

	// Mode-specific setup
	DialoguesEditorPtr->PushTabFactories(DialoguesEditorTabFactories);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FDialoguesEditorApplicationMode::PreDeactivateMode()
{
	FApplicationMode::PreDeactivateMode();

	check(DialoguesEditor.IsValid());
	TSharedPtr<FDialoguesEditor> DialoguesEditorPtr = DialoguesEditor.Pin();

	DialoguesEditorPtr->SaveEditedObjectState();
}

void FDialoguesEditorApplicationMode::PostActivateMode()
{
	check(DialoguesEditor.IsValid());
	TSharedPtr<FDialoguesEditor> DialoguesEditorPtr = DialoguesEditor.Pin();
	DialoguesEditorPtr->RestoreDialogues();

	FApplicationMode::PostActivateMode();
}
