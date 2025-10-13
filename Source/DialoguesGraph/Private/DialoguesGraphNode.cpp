#include "DialoguesGraphNode.h"
#include "Dialogues.h"
#include "DialogueNode.h"
#include "DialoguesGraphTypes.h"
#include "SGraphNode_DialoguesGraph.h"
#include "EdGraphSchema_Dialogues.h"
#include "Kismet2/KismetEditorUtilities.h"

UDialoguesGraphNode::UDialoguesGraphNode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

UDialoguesGraph* UDialoguesGraphNode::GetDialoguesGraph()
{
	return CastChecked<UDialoguesGraph>(GetGraph());
}

void UDialoguesGraphNode::AllocateDefaultPins()
{

}

FText UDialoguesGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (const UDialogueNode* RuntimeNode = Cast<const UDialogueNode>(NodeInstance))
	{
		return RuntimeNode->GetDisplayNameText();
	}

	if (!ClassData.GetClassName().IsEmpty())
	{
		FString StoredClassName = ClassData.GetClassName();
		StoredClassName.RemoveFromEnd(TEXT("_C"));

		return FText::Format(NSLOCTEXT("ConversationGraph", "NodeClassError", "Class {0} not found, make sure it's saved!"), FText::FromString(StoredClassName));
	}

	return Super::GetNodeTitle(TitleType);
}

bool UDialoguesGraphNode::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const
{
	return DesiredSchema->GetClass()->IsChildOf(UEdGraphSchema_Dialogues::StaticClass());
}

FLinearColor UDialoguesGraphNode::GetNodeBodyTintColor() const
{
	return DialogueEditorColors::NodeBody::Default;
}

UObject* UDialoguesGraphNode::GetJumpTargetForDoubleClick() const
{
	return (NodeInstance != nullptr) ? NodeInstance->GetClass() : nullptr;
}

bool UDialoguesGraphNode::CanJumpToDefinition() const
{
	return GetJumpTargetForDoubleClick() != nullptr;
}

void UDialoguesGraphNode::JumpToDefinition() const
{
	if (UObject* HyperlinkTarget = GetJumpTargetForDoubleClick())
	{
		FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(HyperlinkTarget);
	}
}

TSharedPtr<SGraphNode> UDialoguesGraphNode::CreateVisualWidget()
{
	return SNew(SGraphNode_Dialogues, this);
}

FName UDialoguesGraphNode::GetNameIcon() const
{
	if (const UDialogueNode* RuntimeNode = Cast<const UDialogueNode>(NodeInstance))
	{
		return RuntimeNode->GetNodeIconName();
	}

	return FName("Dialogues.Graph.Icon");
}

FText UDialoguesGraphNode::GetDescription() const
{
	const UDialogueNode* DialogueNode = Cast<const UDialogueNode>(NodeInstance);
	if (DialogueNode)
	{
		if (!DialogueNode->ShowPropertyEditors())
			return DialogueNode->GetStaticDescription();
		else
			return FText::GetEmpty();
	}

	return Super::GetDescription();
}

void UDialoguesGraphNode::InitializeInstance()
{
	UDialogueNode* DialogueNode = Cast<UDialogueNode>(NodeInstance);
	UDialogues* BTAsset = DialogueNode ? Cast<UDialogues>(DialogueNode->GetOuter()) : nullptr;
	if (DialogueNode && BTAsset)
	{
		DialogueNode->InitializeFromAsset(*BTAsset);
		DialogueNode->InitializeNode(NULL);
		DialogueNode->OnNodeCreated();
	}
}
