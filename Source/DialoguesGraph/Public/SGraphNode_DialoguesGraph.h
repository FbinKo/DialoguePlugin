#pragma once

//#include "DialogueTreeEditorTypes.h"
#include "Containers/Array.h"
#include "HAL/Platform.h"
#include "Input/Reply.h"
#include "Internationalization/Text.h"
#include "Kismet2/EnumEditorUtils.h"
#include "Layout/Visibility.h"
#include "Math/Color.h"
#include "Math/Vector2D.h"
#include "SGraphNodeAI.h"
#include "SNodePanel.h"
#include "Styling/SlateColor.h"
#include "Templates/SharedPointer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "DialoguesGraphTypes.h"

class UDialoguesGraphNode;

class DIALOGUESGRAPH_API SGraphNode_Dialogues : public SGraphNodeAI
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_Dialogues) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDialoguesGraphNode* InNode);

	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual TSharedPtr<SToolTip> GetComplexTooltip() override;
	//virtual void GetOverlayBrushes(bool bSelected, const FVector2D WidgetSize, TArray<FOverlayBrushInfo>& Brushes) const override;
	virtual TArray<FOverlayWidgetInfo> GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const override;
	virtual TSharedRef<SGraphNode> GetNodeUnderMouse(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	//virtual void MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty = true) override;
	// End of SGraphNode interface

	void AddSubNodeWidget(TSharedPtr<SGraphNode> NewSubNodeWidget, EDialogueGraphSubNodeWidgetLocation Location);

//	/** handle double click */
//	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	//	/** shows red marker when search failed*/
//	EVisibility GetDebuggerSearchFailedMarkerVisibility() const;
//
//	FVector2D GetCachedPosition() const { return CachedPosition; }

protected:
	uint32 bSuppressDebuggerTriggers : 1;

	float DebuggerStateDuration;

	int32 DebuggerStateCounter;

	TMap<EDialogueGraphSubNodeWidgetLocation, FSubNodeWidgetInfo> ChildNodeInfo;
//
//	/** debugger colors */
//	FLinearColor FlashColor;
//	float FlashAlpha;
//
//	/** height offsets for search triggers */
//	TArray<FNodeBounds> TriggerOffsets;
//
//	/** cached draw position */
//	FVector2D CachedPosition;

	TSharedPtr<SHorizontalBox> OutputPinBox;

	/** The node body widget, cached here so we can determine its size when we want ot position our overlays */
	TSharedPtr<SBorder> NodeBody;

	FSlateColor GetBorderBackgroundColor() const;
	FSlateColor GetBackgroundColor() const;

	virtual const FSlateBrush* GetNameIcon() const override;
	virtual EVisibility GetBlueprintIconVisibility() const;

	FText GetPinTooltip(UEdGraphPin* GraphPinObj) const;

	TSharedPtr<class IPropertyRowGenerator> PropertyRowGenerator;

	void PropertyRowsRefreshed();

	class SVerticalBox::FSlot* PropertyDetailsSlot;

	virtual bool CanReachNode() const;
};