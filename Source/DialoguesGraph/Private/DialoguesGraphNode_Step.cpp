#include "DialoguesGraphNode_Step.h"
#include "DialogueNode_Step.h"
#include "DialoguesGraphTypes.h"
#include "SGraphEditorActionMenuAI.h"

#define LOCTEXT_NAMESPACE "DialoguesGraph"

UDialoguesGraphNode_Step::UDialoguesGraphNode_Step(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UDialoguesGraphNode_Step::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UDialogueGraphTypes::PinCategory_MultipleNodes, TEXT("In"));
	CreatePin(EGPD_Output, UDialogueGraphTypes::PinCategory_MultipleNodes, TEXT("Out"));
}

FText UDialoguesGraphNode_Step::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (const UDialogueNode* MyNode = Cast<UDialogueNode>(NodeInstance))
	{
		return MyNode->GetDisplayNameText();
	}

	return Super::GetNodeTitle(TitleType);
}

FLinearColor UDialoguesGraphNode_Step::GetNodeBodyTintColor() const
{
	FLinearColor Result = DialogueEditorColors::NodeBody::Default;
	if (NodeInstance)
	{
		UDialogueNode_Step* Step = GetRuntimeNode<UDialogueNode_Step>();
		if (!Step->GetNodeBodyColor(/*out*/ Result))
		{
			if(Step->EntryTag.IsValid())
				Result = DialogueEditorColors::NodeBody::Entry;
			else if(Step->bIsDialogueExit)
				Result = DialogueEditorColors::NodeBody::Exit;
		}
	}
	return Result;
}

void UDialoguesGraphNode_Step::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	TWeakObjectPtr<UDialoguesGraphNode> WeakThis(const_cast<UDialoguesGraphNode_Step*>(this));

	auto CreateAddRequirementSubMenu = [WeakThis](FMenuBuilder& MenuBuilder, UEdGraph* Graph)
		{
			TSharedRef<SGraphEditorActionMenuAI> Menu =
				SNew(SGraphEditorActionMenuAI)
				.GraphObj(Graph)
				.GraphNode(WeakThis.Get())
				.SubNodeFlags((int32)EDialogueGraphSubNodeType::Requirement)
				.AutoExpandActionMenu(true);

			MenuBuilder.AddWidget(Menu, FText(), true);
		};

	FToolMenuSection& Section = Menu->FindOrAddSection("DialogueGraphNode_Step");

	Section.AddSubMenu(
		"AddRequirement",
		LOCTEXT("AddRequirement", "Add Requirement..."),
		LOCTEXT("AddRequirementTooltip", "Adds a new requirement as a subnode"),
		FNewMenuDelegate::CreateLambda(CreateAddRequirementSubMenu, (UEdGraph*)Context->Graph));
}
#undef LOCTEXT_NAMESPACE