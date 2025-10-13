#pragma once

#include "DialoguesGraphNode.h"
#include "DialoguesGraphNode_Requirement.generated.h"

class UEdGraph;
class UEdGraphSchema;

UCLASS()
class DIALOGUESGRAPH_API UDialoguesGraphNode_Requirement : public UDialoguesGraphNode
{
	GENERATED_UCLASS_BODY()

	virtual FLinearColor GetNodeBodyTintColor() const override;
};
