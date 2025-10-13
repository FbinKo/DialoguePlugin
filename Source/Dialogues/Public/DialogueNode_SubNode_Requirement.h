#pragma once

#include "CoreMinimal.h"
#include "DialogueNode_SubNode.h"
#include "DialogueNode_SubNode_Requirement.generated.h"

UCLASS(Abstract, Blueprintable)
class DIALOGUES_API UDialogueNode_SubNode_Requirement : public UDialogueNode_SubNode
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	ERequirementResult IsRequirementSatisfied() const;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif
};