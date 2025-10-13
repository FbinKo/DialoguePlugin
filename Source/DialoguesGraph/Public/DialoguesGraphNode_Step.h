#pragma once

#include "DialoguesGraphNode.h"
#include "GameplayTagContainer.h"
#include "DialoguesGraphNode_Step.generated.h"

class UEdGraph;
class UEdGraphSchema;

UCLASS()
class DIALOGUESGRAPH_API UDialoguesGraphNode_Step : public UDialoguesGraphNode
{
	GENERATED_UCLASS_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeBodyTintColor() const override;

	/** Gets a list of actions that can be done to this particular node */
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;

	virtual bool CanPlaceBreakpoints() const override { return true; }
};
