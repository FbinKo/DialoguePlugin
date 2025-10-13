#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"

class UEdGraphNode;

/** Dialogues Editor public interface */
class IDialoguesEditor : public FWorkflowCentricApplication
{

public:
	//virtual void InitializeDebuggerState(class FDialoguesDebugger* ParentDebugger) const = 0;
	//virtual UEdGraphNode* FindInjectedNode(int32 Index) const = 0;
	virtual void DoubleClickNode(class UEdGraphNode* Node) = 0;
};