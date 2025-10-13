#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "DialogueNode.generated.h"

class UDialogues;
class UDialoguesComponent;

UCLASS(Abstract, Const)
class DIALOGUES_API UDialogueNode : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	virtual UWorld* GetWorld() const override;

	virtual void InitializeNode(UDialogueNode* InParentNode);

	virtual void InitializeFromAsset(UDialogues& Asset);

	virtual FText GetDisplayNameText() const;

	virtual void DescribeRuntimeValues(const UDialoguesComponent& OwnerComp, bool bDetailedValues, TArray<FString>& Values) const;

	FText GetRuntimeDescription(const UDialoguesComponent& OwnerComp, bool bDetailedDescription) const;

	virtual FText GetStaticDescription() const;

	UFUNCTION(BlueprintNativeEvent)
	bool GetNodeBodyColor(FLinearColor& BodyColor) const;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const;

	/** Called after creating new node in dialogue editor, use for versioning */
	virtual void OnNodeCreated() {}

	bool ShowPropertyEditors() const
	{
		return bShowPropertyEditors;
	}

	bool ShowPropertyDetails() const
	{
		if (bShowPropertyEditors)
		{
			return false;
		}

		return bShowPropertyDetails;
	}
#endif

protected:
#if WITH_EDITOR
	bool ShouldHideProperty(FProperty* InTestProperty) const;
#endif // #if WITH_EDITOR

protected:
	UPROPERTY(EditAnywhere, Category = Description, AdvancedDisplay)
	FString NodeName;

	UPROPERTY()
	FGuid Compiled_NodeGUID;

#if WITH_EDITORONLY_DATA
	/** show detailed information about properties */
	UPROPERTY(EditInstanceOnly, Category = Description, AdvancedDisplay)
	uint32 bShowPropertyDetails : 1;

	/** show detailed information about properties */
	UPROPERTY(EditInstanceOnly, Category = Description, AdvancedDisplay)
	uint32 bShowPropertyEditors : 1;
#endif
};