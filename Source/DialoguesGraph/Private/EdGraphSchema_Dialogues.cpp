#include "EdGraphSchema_Dialogues.h"
#include "DialoguesGraphNode.h"
#include "DialoguesGraphNode_Step.h"
#include "DialoguesGraphNode_Requirement.h"
#include "DialogueNode_Step.h"
#include "DialogueNode_SubNode_Requirement.h"
#include "DialoguesGraphConnectionDrawingPolicy.h"
#include "DialoguesGraphTypes.h"
#include "AIGraphSchema.h"

#define LOCTEXT_NAMESPACE "DialoguesGraph"

int32 UEdGraphSchema_Dialogues::CurrentCacheRefreshID = 0;

TSharedPtr<FGraphNodeClassHelper> ClassCache;

FGraphNodeClassHelper* GetClassCache()
{
	if (!ClassCache.IsValid())
	{
		ClassCache = MakeShareable(new FGraphNodeClassHelper(UDialogueNode::StaticClass()));
		FGraphNodeClassHelper::AddObservedBlueprintClasses(UDialogueNode_Step::StaticClass());

		ClassCache->UpdateAvailableBlueprintClasses();
	}

	return ClassCache.Get();
}

UEdGraphSchema_Dialogues::UEdGraphSchema_Dialogues(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UEdGraphSchema_Dialogues::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{

}

void UEdGraphSchema_Dialogues::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	const FName PinCategory = ContextMenuBuilder.FromPin ?
		ContextMenuBuilder.FromPin->PinType.PinCategory :
		UDialogueGraphTypes::PinCategory_MultipleNodes;

	const bool bNoParent = (ContextMenuBuilder.FromPin == NULL);

	FGraphNodeClassHelper* GraphClassCache = GetClassCache();

	{
		FCategorizedGraphActionListBuilder StepsBuilder(TEXT("Steps"));

		TArray<FGraphNodeClassData> NodeClasses;
		GraphClassCache->GatherClasses(UDialogueNode_Step::StaticClass(), NodeClasses);

		for (const auto& NodeClass : NodeClasses)
		{
			const FText NodeTypeName = FText::FromString(FName::NameToDisplayString(NodeClass.ToString(), false));

			TSharedPtr<FAISchemaAction_NewNode> AddOpAction = TSharedPtr<FAISchemaAction_NewNode>(new FAISchemaAction_NewNode(NodeClass.GetCategory(), NodeTypeName, FText::GetEmpty(), 0));
			StepsBuilder.AddAction(AddOpAction);

			UClass* GraphNodeClass = UDialoguesGraphNode_Step::StaticClass();

			UDialoguesGraphNode* OpNode = NewObject<UDialoguesGraphNode>(ContextMenuBuilder.OwnerOfTemporaries, GraphNodeClass);
			OpNode->ClassData = NodeClass;
			AddOpAction->NodeTemplate = OpNode;
		}

		ContextMenuBuilder.Append(StepsBuilder);
	}
}

void UEdGraphSchema_Dialogues::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (Context->Node && !Context->Pin)
	{
		const UDialoguesGraphNode* BTGraphNode = Cast<const UDialoguesGraphNode>(Context->Node);
		if (BTGraphNode/* && BTGraphNode->CanPlaceBreakpoints()*/)
		{
			/*FToolMenuSection& Section = Menu->AddSection("EdGraphSchemaBreakpoints", LOCTEXT("BreakpointsHeader", "Breakpoints"));
			{
				Section.AddMenuEntry(FGraphEditorCommands::Get().ToggleBreakpoint);
				Section.AddMenuEntry(FGraphEditorCommands::Get().AddBreakpoint);
				Section.AddMenuEntry(FGraphEditorCommands::Get().RemoveBreakpoint);
				Section.AddMenuEntry(FGraphEditorCommands::Get().EnableBreakpoint);
				Section.AddMenuEntry(FGraphEditorCommands::Get().DisableBreakpoint);
			}*/
		}
	}

	Super::GetContextMenuActions(Menu, Context);
}

const FPinConnectionResponse UEdGraphSchema_Dialogues::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	// Make sure the pins are not on the same node
	if (A->GetOwningNode() == B->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorSameNode", "Both are on the same node"));
	}

	const bool bPinAIsSingleComposite = (A->PinType.PinCategory == UDialogueGraphTypes::PinCategory_SingleComposite);
	const bool bPinAIsSingleStep = (A->PinType.PinCategory == UDialogueGraphTypes::PinCategory_SingleStep);
	const bool bPinAIsSingleNode = (A->PinType.PinCategory == UDialogueGraphTypes::PinCategory_SingleNode);

	const bool bPinBIsSingleComposite = (B->PinType.PinCategory == UDialogueGraphTypes::PinCategory_SingleComposite);
	const bool bPinBIsSingleStep = (B->PinType.PinCategory == UDialogueGraphTypes::PinCategory_SingleStep);
	const bool bPinBIsSingleNode = (B->PinType.PinCategory == UDialogueGraphTypes::PinCategory_SingleNode);

	const bool bPinAIsStep = A->GetOwningNode()->IsA(UDialoguesGraphNode_Step::StaticClass());

	const bool bPinBIsStep = B->GetOwningNode()->IsA(UDialoguesGraphNode_Step::StaticClass());

	if (bPinAIsSingleComposite || bPinBIsSingleComposite)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOnlyComposite", "Only composite nodes are allowed"));
	}

	if ((bPinAIsSingleStep && !bPinBIsStep) || (bPinBIsSingleStep && !bPinAIsStep))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOnlyStep", "Only Step nodes are allowed"));
	}

	// Compare the directions
	if ((A->Direction == EGPD_Input) && (B->Direction == EGPD_Input))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorInput", "Can't connect input node to input node"));
	}
	else if ((B->Direction == EGPD_Output) && (A->Direction == EGPD_Output))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOutput", "Can't connect output node to output node"));
	}

	class FNodeVisitorCycleChecker
	{
	public:
		/** Check whether a loop in the graph would be caused by linking the passed-in nodes */
		bool CheckForLoop(UEdGraphNode* StartNode, UEdGraphNode* EndNode)
		{
			VisitedNodes.Add(EndNode);
			return TraverseInputNodesToRoot(StartNode);
		}

	private:
		/**
		 * Helper function for CheckForLoop()
		 * @param	Node	The node to start traversal at
		 * @return true if we reached a root node (i.e. a node with no input pins), false if we encounter a node we have already seen
		 */
		bool TraverseInputNodesToRoot(UEdGraphNode* Node)
		{
			VisitedNodes.Add(Node);

			// Follow every input pin until we cant any more ('root') or we reach a node we have seen (cycle)
			for (int32 PinIndex = 0; PinIndex < Node->Pins.Num(); ++PinIndex)
			{
				UEdGraphPin* MyPin = Node->Pins[PinIndex];

				if (MyPin->Direction == EGPD_Input)
				{
					for (int32 LinkedPinIndex = 0; LinkedPinIndex < MyPin->LinkedTo.Num(); ++LinkedPinIndex)
					{
						UEdGraphPin* OtherPin = MyPin->LinkedTo[LinkedPinIndex];
						if (OtherPin)
						{
							UEdGraphNode* OtherNode = OtherPin->GetOwningNode();
							if (VisitedNodes.Contains(OtherNode))
							{
								return false;
							}
							else
							{
								return TraverseInputNodesToRoot(OtherNode);
							}
						}
					}
				}
			}

			return true;
		}

		TSet<UEdGraphNode*> VisitedNodes;
	};

	// check for cycles
	FNodeVisitorCycleChecker CycleChecker;
	if (!CycleChecker.CheckForLoop(A->GetOwningNode(), B->GetOwningNode()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorcycle", "Can't create a graph cycle"));
	}

	const bool bPinASingleLink = bPinAIsSingleComposite || bPinAIsSingleStep || bPinAIsSingleNode;
	const bool bPinBSingleLink = bPinBIsSingleComposite || bPinBIsSingleStep || bPinBIsSingleNode;

	if (bPinASingleLink && A->LinkedTo.Num() > 0)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("PinConnectReplace", "Replace connection"));
	}
	else if (bPinBSingleLink && B->LinkedTo.Num() > 0)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, LOCTEXT("PinConnectReplace", "Replace connection"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes"));
}

const FPinConnectionResponse UEdGraphSchema_Dialogues::CanMergeNodes(const UEdGraphNode* A, const UEdGraphNode* B) const
{
	// Make sure the nodes are not the same 
	if (A == B)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both are the same node"));
	}

	const bool bNodeAIsRequirement = A->IsA(UDialoguesGraphNode_Requirement::StaticClass());
	const bool bNodeBIsRequirement = B->IsA(UDialoguesGraphNode_Requirement::StaticClass());
	const bool bNodeBIsStep = B->IsA(UDialoguesGraphNode_Step::StaticClass());

	if ((bNodeAIsRequirement && (bNodeBIsStep || bNodeBIsRequirement)))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT(""));
}

FLinearColor UEdGraphSchema_Dialogues::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FColor::White;
}

FConnectionDrawingPolicy* UEdGraphSchema_Dialogues::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
	return new FDialoguesGraphConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

bool UEdGraphSchema_Dialogues::IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const
{
	return CurrentCacheRefreshID != InVisualizationCacheID;
}

int32 UEdGraphSchema_Dialogues::GetCurrentVisualizationCacheID() const
{
	return CurrentCacheRefreshID;
}

void UEdGraphSchema_Dialogues::ForceVisualizationCacheClear() const
{
	++CurrentCacheRefreshID;
}

void UEdGraphSchema_Dialogues::GetSubNodeClasses(int32 SubNodeFlags, TArray<FGraphNodeClassData>& ClassData, UClass*& GraphNodeClass) const
{
	FGraphNodeClassHelper* GraphClassCache = GetClassCache();

	switch ((EDialogueGraphSubNodeType)SubNodeFlags)
	{
	case EDialogueGraphSubNodeType::Requirement:
		GraphClassCache->GatherClasses(UDialogueNode_SubNode_Requirement::StaticClass(), ClassData);
		GraphNodeClass = UDialoguesGraphNode_Requirement::StaticClass();
	default:
		break;
	}
}

#undef LOCTEXT_NAMESPACE