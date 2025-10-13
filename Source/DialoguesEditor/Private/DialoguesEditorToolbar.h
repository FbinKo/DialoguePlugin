#pragma once

#include "Templates/SharedPointer.h"

class FDialoguesEditor;
class FExtender;
class FToolBarBuilder;

class FDialoguesEditorToolbar : public TSharedFromThis<FDialoguesEditorToolbar>
{
public:
	FDialoguesEditorToolbar(TSharedPtr<FDialoguesEditor> InDialoguesEditor)
		: DialoguesEditor(InDialoguesEditor) {}

	void AddModesToolbar(TSharedPtr<FExtender> Extender);
	//void AddDebuggerToolbar(TSharedPtr<FExtender> Extender);
	void AddDialoguesToolbar(TSharedPtr<FExtender> Extender);

private:
	void FillModesToolbar(FToolBarBuilder& ToolbarBuilder);
	//void FillDebuggerToolbar(FToolBarBuilder& ToolbarBuilder);
	void FillDialoguesToolbar(FToolBarBuilder& ToolbarBuilder);

protected:
	/** Pointer back to the blueprint editor tool that owns us */
	TWeakPtr<FDialoguesEditor> DialoguesEditor;
};