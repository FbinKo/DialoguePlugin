#pragma once

#include "Templates/SharedPointer.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

/** Application mode for main behavior tree editing mode */
class FDialoguesEditorApplicationMode : public FApplicationMode
{
public:
	FDialoguesEditorApplicationMode(TSharedPtr<class FDialoguesEditor> InDialoguesEditor);

	virtual void RegisterTabFactories(TSharedPtr<class FTabManager> InTabManager) override;
	virtual void PreDeactivateMode() override;
	virtual void PostActivateMode() override;

protected:
	TWeakPtr<class FDialoguesEditor> DialoguesEditor;

	// Set of spawnable tabs in behavior tree editing mode
	FWorkflowAllowedTabSet DialoguesEditorTabFactories;
};