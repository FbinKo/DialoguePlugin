#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DialogueNode.h"
#include "DialogueNode_SubNode_Requirement.h"
#include "DialoguesTypes.h"
#include "DialogueNode_Step.generated.h"

class UDialogueNode_SubNode;

UCLASS(Blueprintable)
class DIALOGUES_API UDialogueNode_Step : public UDialogueNode
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(meta = (HideInDialogueNode))
	bool bCanBeReached;

	UPROPERTY()
	TArray<TObjectPtr<UDialogueNode_SubNode>> SubNodes;

	UPROPERTY()
	TArray<TObjectPtr<UDialogueNode_Step>> PreviousSteps;

	UPROPERTY()
	TArray<TObjectPtr<UDialogueNode_Step>> NextSteps;

	UPROPERTY(EditAnywhere, Category = "Link", meta = (HideInDialogueNode))
	FGameplayTag EntryTag;

protected:
	UPROPERTY(EditAnywhere, Category = "Link", meta = (HideInDialogueNode))
	bool bContinuesAutomatically;

	/*
	* if false, advance dialogue will move up to last choice
	*/
	UPROPERTY(EditAnywhere, Category = "Link", meta = (HideInDialogueNode))
	bool bIsDialogueExit;

	UPROPERTY(EditAnywhere, Category = "Content")
	FGameplayTag SpeakerID;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Content", meta = (HideInDialogueNode))
	FText Title = NSLOCTEXT("DialogueOption", "DialogueOption_DefaultText", "Continue");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
	TObjectPtr<UDialogueData> Data;

public:
	virtual void InitializeNode(UDialogueNode* InParentNode) override;
	bool ContinuesAutomatically() { return bContinuesAutomatically; }
	bool IsDialogueExit() { return NextSteps.IsEmpty() && bIsDialogueExit; }

	virtual FText GetDisplayNameText() const override;

	UFUNCTION(BlueprintNativeEvent)
	FDialogueStepResult ExecuteStep(const FDialoguesContext& Context) const;

	UFUNCTION(BlueprintNativeEvent)
	ERequirementResult CheckRequirements(const FDialoguesContext& Context) const;

	TArray<UDialogueNode_Step*> GetOptions() const { return NextSteps; }

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif

	friend class UDialoguesGraphNode_Step;
};