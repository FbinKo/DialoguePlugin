#include "DialoguesGraph.h"
#include "Dialogues.h"
#include "DialoguesGraphNode.h"
#include "DialoguesGraphNode_Step.h"
#include "DialogueNode_Step.h"
#include "DialogueNode_SubNode.h"
#include "EdGraphSchema_Dialogues.h"

UDialoguesGraph::UDialoguesGraph(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Schema = UEdGraphSchema_Dialogues::StaticClass();
}

void UDialoguesGraph::OnSave()
{
	UpdateAsset();
}

void UDialoguesGraph::UpdateAsset(int32 UpdateFlags)
{
	if (bLockUpdates)
	{
		return;
	}

	// set ParentNode for GraphNodeSubNodes
	for (UEdGraphNode* Node : Nodes)
	{
		if (UAIGraphNode* AINode = Cast<UAIGraphNode>(Node))
		{
			for (UAIGraphNode* SubNode : AINode->SubNodes)
			{
				if (SubNode)
				{
					SubNode->ParentNode = AINode;
				}
			}
		}
	}

	// we can't look at pins until pin references have been fixed up post undo:
	UEdGraphPin::ResolveAllPinReferences();

	CreateDialoguesFromGraph();
}

void UDialoguesGraph::CreateDialoguesFromGraph()
{
	UDialogues* DialoguesAsset = CastChecked<UDialogues>(GetOuter());
	DialoguesAsset->EntryNodeMap.Reset();

	TMap<FGameplayTag, FDialogueEntryList> EntryNodes;
	TArray<UDialoguesGraphNode_Step*> AllSteps;
	DialoguesAsset->Graph->GetNodesOfClass<UDialoguesGraphNode_Step>(AllSteps);

	for (UDialoguesGraphNode_Step* Step : AllSteps)
	{
		if (Step && !Step->HasErrors())
		{
			if(UDialogueNode_Step* instance = Cast<UDialogueNode_Step>(Step->NodeInstance))
			{
				instance->InitializeNode(nullptr);
				instance->SubNodes.Reset();
				for (UAIGraphNode* GraphSubNode : Step->SubNodes)
				{
					if (UDialoguesGraphNode* DialogueGraphSubNode = Cast<UDialoguesGraphNode>(GraphSubNode))
					{
						if(UDialogueNode_SubNode* SubNode = DialogueGraphSubNode->GetRuntimeNode<UDialogueNode_SubNode>())
						{
							instance->SubNodes.Add(SubNode);
							SubNode->InitializeNode(instance);
						}
					}
				}

				instance->PreviousSteps.Reset();
				for (UEdGraphPin* PreviousStepPin : Step->GetInputPin()->LinkedTo)
				{
					if (UDialoguesGraphNode_Step* previousStep = Cast<UDialoguesGraphNode_Step>(PreviousStepPin->GetOwningNode()))
					{
						if (UDialogueNode_Step* previousInstance = Cast<UDialogueNode_Step>(previousStep->NodeInstance))
						{
							instance->InitializeNode(previousInstance);
						}
					}
				}

				instance->NextSteps.Reset();
				for (UEdGraphPin* NextStepPin : Step->GetOutputPin()->LinkedTo)
				{
					if (UDialoguesGraphNode_Step* nextStep = Cast<UDialoguesGraphNode_Step>(NextStepPin->GetOwningNode()))
					{
						if (UDialogueNode_Step* nextInstance = Cast<UDialogueNode_Step>(nextStep->NodeInstance))
						{
							instance->NextSteps.Add(nextInstance);
						}
					}
				}

				EntryNodes.FindOrAdd(instance->EntryTag).Add(instance);
			}
		}
	}
	DialoguesAsset->EntryNodeMap.Append(EntryNodes);
}

#if WITH_EDITOR
void UDialoguesGraph::PostEditUndo()
{
	Super::PostEditUndo();

	// make sure that all execution indices are up to date
	UpdateAsset(/*KeepRebuildCounter*/);
	Modify();
}
#endif