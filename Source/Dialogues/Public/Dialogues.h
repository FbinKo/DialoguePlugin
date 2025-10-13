#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/Blueprint.h"
#include "GameplayTagContainer.h"
#include "Dialogues.generated.h"

class UDialogueNode_Step;

USTRUCT()
struct FDialogueEntryList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UDialogueNode_Step>> DestinationList;

	void Add(UDialogueNode_Step* NewStep) {
		DestinationList.Add(NewStep);
	}
};

UCLASS(BlueprintType)
class DIALOGUES_API UDialogues : public UObject
{
	GENERATED_UCLASS_BODY()

	public:
#if WITH_EDITORONLY_DATA

	/** Graph for Dialogues */
	UPROPERTY()
	TObjectPtr<class UEdGraph>	Graph;

	/** Info about the graphs we last edited */
	UPROPERTY()
	TArray<FEditedDocumentInfo> LastEditedDocuments;
#endif

	TMap<FGameplayTag, FDialogueEntryList> EntryNodeMap;
};