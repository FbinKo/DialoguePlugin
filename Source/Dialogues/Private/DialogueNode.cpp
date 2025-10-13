#include "DialogueNode.h"
#include "BlueprintNodeHelpers.h"

#define LOCTEXT_NAMESPACE "DialoguesEditor"

UDialogueNode::UDialogueNode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShowPropertyDetails = true;
#endif
}

UWorld* UDialogueNode::GetWorld() const
{
	if (GetOuter() == nullptr)
	{
		return nullptr;
	}

	// Special case for nodes in the editor
	if (Cast<UPackage>(GetOuter()) != nullptr)
	{
		// GetOuter should return a UPackage and its Outer is a UWorld
		return Cast<UWorld>(GetOuter()->GetOuter());
	}

	// In all other cases...
	return GetOuter()->GetWorld();
}

void UDialogueNode::InitializeNode(UDialogueNode* InParentNode)
{
}

void UDialogueNode::InitializeFromAsset(UDialogues& Asset)
{
}

FText UDialogueNode::GetDisplayNameText() const
{
	return NodeName.Len() ?
		FText::FromString(NodeName)
		:
#if WITH_EDITOR
		GetClass()->GetDisplayNameText();
#else
		FText::FromName(GetClass()->GetFName());
#endif
}

void UDialogueNode::DescribeRuntimeValues(const UDialoguesComponent& OwnerComp, bool bDetailedValues, TArray<FString>& Values) const
{
}

FText UDialogueNode::GetRuntimeDescription(const UDialoguesComponent& OwnerComp, bool bDetailedDescription) const
{
	TArray<FString> RuntimeValues;
	DescribeRuntimeValues(OwnerComp, bDetailedDescription, RuntimeValues);

	return FText::Format(LOCTEXT("NodeRuntimeDescription", "{0} [{1}]{2}"),
		GetDisplayNameText(),
		GetStaticDescription(),
		FText::FromString(FString::Join(RuntimeValues, TEXT(", ")))
	);
}

FText UDialogueNode::GetStaticDescription() const
{
#if WITH_EDITOR
	UDialogueNode* CDO = GetClass()->GetDefaultObject<UDialogueNode>();
	if (ShowPropertyDetails() && CDO)
	{
		const UClass* StopAtClass = UDialogueNode::StaticClass();
		const FString PropertyDesc = BlueprintNodeHelpers::CollectPropertyDescription(this, StopAtClass, [this](FProperty* InTestProperty)
			{
				return ShouldHideProperty(InTestProperty);
			});

		return FText::FromString(PropertyDesc);
	}
#endif // WITH_EDITOR

	return GetDisplayNameText();
}

bool UDialogueNode::GetNodeBodyColor_Implementation(FLinearColor& BodyColor) const
{
	return false;
}

#if WITH_EDITOR
FName UDialogueNode::GetNodeIconName() const
{
	return NAME_None;
}

bool UDialogueNode::ShouldHideProperty(FProperty* InTestProperty) const
{
	const static FName HideInDialogueMeta{ TEXT("HideInDialogueNode") };
	if (InTestProperty->HasMetaData(HideInDialogueMeta))
	{
		return true;
	}
	// base property for single structs
	else if (FStructProperty* StructTestProperty = CastField<FStructProperty>(InTestProperty))
	{
		return StructTestProperty->Struct->HasMetaData(HideInDialogueMeta);
	}
	// base property for strong/soft object pointers
	else if (FObjectPropertyBase* ObjectTestProperty = CastField<FObjectPropertyBase>(InTestProperty))
	{
		return ObjectTestProperty->PropertyClass->HasMetaData(HideInDialogueMeta);
	}
	// container support for this would be cleanest if we pulled everything but the first if into a helper, eg ShouldHideProp(ArrayProperty->Element)
	return false;
}
#endif

#undef LOCTEXT_NAMESPACE