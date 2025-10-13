#include "DialoguesGraphNode_Requirement.h"
#include "DialoguesGraphTypes.h"

UDialoguesGraphNode_Requirement::UDialoguesGraphNode_Requirement(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

FLinearColor UDialoguesGraphNode_Requirement::GetNodeBodyTintColor() const
{
	return DialogueEditorColors::NodeBody::RequirementColor;
}