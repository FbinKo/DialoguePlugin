#pragma once

#include "CoreMinimal.h"
#include "DialoguesGraphTypes.generated.h"

namespace DialogueEditorColors
{
	namespace NodeBody
	{
		const FLinearColor RequirementColor(0.0f, 0.07f, 0.4f);

		const FLinearColor Default(0.15f, 0.15f, 0.15f);
		const FLinearColor Entry(0.f, 0.7f, 0.f);
		const FLinearColor Exit(0.7f, 0.7f, 0.f);
		const FLinearColor Error(1.0f, 0.0f, 0.0f);
	}

	namespace NodeBorder
	{
		const FLinearColor Inactive(0.08f, 0.08f, 0.08f);
		const FLinearColor Selected(1.00f, 0.58f, 0.08f);
		const FLinearColor Disconnected(0.f, 0.f, 0.f);
		const FLinearColor ActiveDebugging(1.0f, 1.0f, 0.0f);
		const FLinearColor InactiveDebugging(0.4f, 0.4f, 0.0f);
		const FLinearColor QuickFind(0.f, 0.8f, 0.f);
	}

	namespace Pin
	{
		const FLinearColor Diff(0.9f, 0.2f, 0.15f);
		const FLinearColor Hover(1.0f, 0.7f, 0.0f);
		const FLinearColor Default(0.02f, 0.02f, 0.02f);
		const FLinearColor StepOnly(0.13f, 0.03f, 0.4f);
		const FLinearColor CompositeOnly(0.02f, 0.02f, 0.02f);
		const FLinearColor SingleNode(0.02f, 0.02f, 0.02f);
	}

	namespace Connection
	{
		const FLinearColor Default(1.0f, 1.0f, 1.0f);
	}

	namespace Debugger
	{
		const FLinearColor StepFlash(1.0f, 1.0f, 0.0f);

		const FLinearColor DescHeader(0.3f, 0.8f, 0.4f);
		const FLinearColor DescKeys(0.3f, 0.4f, 0.8f);
	}

	namespace Action
	{
		const FLinearColor DragMarker(1.0f, 1.0f, 0.2f);
	}
}

struct FSubNodeWidgetInfo
{
	TSharedPtr<SVerticalBox> ChildNodeBox;

	TArray< TSharedPtr<SGraphNode> > ChildNodeWidgets;
};

enum class EDialogueGraphSubNodeType : uint8
{
	Requirement
};

enum class EDialogueGraphSubNodeWidgetLocation : uint8
{
	Above,
	Below
};

UCLASS()
class DIALOGUESGRAPH_API UDialogueGraphTypes : public UObject
{
	GENERATED_UCLASS_BODY()

	static const FName PinCategory_MultipleNodes;
	static const FName PinCategory_SingleComposite;
	static const FName PinCategory_SingleStep;
	static const FName PinCategory_SingleNode;
};