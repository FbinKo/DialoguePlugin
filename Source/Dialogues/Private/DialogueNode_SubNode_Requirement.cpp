#include "DialogueNode_SubNode_Requirement.h"
#include "DialoguesTypes.h"

UDialogueNode_SubNode_Requirement::UDialogueNode_SubNode_Requirement(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

ERequirementResult UDialogueNode_SubNode_Requirement::IsRequirementSatisfied_Implementation() const
{
	return ERequirementResult::FailedAndHidden;
}

#if WITH_EDITOR
FName UDialogueNode_SubNode_Requirement::GetNodeIconName() const
{
	return FName("Dialogues.Graph.Node.Requirement.Icon");
}
#endif