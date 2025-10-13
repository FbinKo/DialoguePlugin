#include "DialogueNode_Step.h"
#include "DialoguesTypes.h"
#include "DialogueNode_SubNode_Requirement.h"
#include "BlueprintNodeHelpers.h"

UDialogueNode_Step::UDialogueNode_Step(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bContinuesAutomatically = false;
	bIsDialogueExit = false;
}

FDialogueStepResult UDialogueNode_Step::ExecuteStep_Implementation(const FDialoguesContext& Context) const
{
	FDialogueStepResult result;
	FDialogueMessage message;
	message.SpeakerID = SpeakerID;
	message.Data = Data;
	message.bContinuesAutomatically = bContinuesAutomatically;
	result = FDialogueStepResult::Pause(message);

	return result;
}

ERequirementResult UDialogueNode_Step::CheckRequirements_Implementation(const FDialoguesContext& Context) const
{
	ERequirementResult result = ERequirementResult::Passed;
	for (UDialogueNode_SubNode* subNode : SubNodes)
	{
		if(UDialogueNode_SubNode_Requirement* requirement = Cast<UDialogueNode_SubNode_Requirement>(subNode))
		{
			ERequirementResult newResult = requirement->IsRequirementSatisfied();
			if (result < newResult)
				result = newResult;
		}

	}
	return result;
}

void UDialogueNode_Step::InitializeNode(UDialogueNode* InParentNode)
{
	if (InParentNode)
	{
		if (UDialogueNode_Step* ParentStep = Cast<UDialogueNode_Step>(InParentNode))
		{
			PreviousSteps.AddUnique(ParentStep);
			bCanBeReached |= ParentStep->bCanBeReached;
		}
	}
	else
		bCanBeReached = EntryTag.IsValid();
}

FText UDialogueNode_Step::GetDisplayNameText() const
{
	if (NodeName.Len())
		return FText::FromString(NodeName);
	else {
		FText NameText = EntryTag.IsValid() ?
			FText::FromString(EntryTag.ToString())
			:
#if WITH_EDITOR
			GetClass()->GetDisplayNameText();
#else
			FText::FromName(GetClass()->GetFName());
#endif

		return FText::Format(FText::FromString(TEXT("{0}: {1}")), NameText, Title);
	}
}

#if WITH_EDITOR
FName UDialogueNode_Step::GetNodeIconName() const
{
	return FName("Dialogues.Graph.Node.Step.Icon");
}
#endif