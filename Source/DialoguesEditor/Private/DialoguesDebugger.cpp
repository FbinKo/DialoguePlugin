#include "DialoguesDebugger.h"

FDialoguesDebugger::FDialoguesDebugger()
{
}

FDialoguesDebugger::~FDialoguesDebugger()
{
}

void FDialoguesDebugger::Tick(float DeltaTime)
{
}

bool FDialoguesDebugger::IsTickable() const
{
	return false;
}

void FDialoguesDebugger::Setup(UDialogues* InTreeAsset, TSharedRef<class FDialoguesEditor> InEditorOwner)
{
	EditorOwner = InEditorOwner;
	TreeAsset = InTreeAsset;
	DebuggerInstanceIndex = INDEX_NONE;
	ActiveStepIndex = 0;
	//LastValidStepId = INDEX_NONE;
	ActiveBreakpoints.Reset();

//#if USE_CONVERSATION_DEBUGGER
//	if (IsPIESimulating())
//	{
//		OnBeginPIE(GEditor->bIsSimulatingInEditor);
//
//		Refresh();
//	}
//#endif
}
