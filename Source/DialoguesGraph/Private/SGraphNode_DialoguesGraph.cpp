#include "SGraphNode_DialoguesGraph.h"
#include "DialogueNode.h"
#include "DialogueNode_Step.h"
#include "DialoguesGraphNode.h"
#include "DialoguesGraphNode_Step.h"
#include "DialoguesGraphNode_Requirement.h"
#include "DialoguesStyle.h"
#include "IDetailTreeNode.h"
#include "IDocumentation.h"
#include "IPropertyRowGenerator.h"
#include "GraphEditorSettings.h"
#include "NodeFactory.h"
#include "SCommentBubble.h"
#include "SGraphPanel.h"
#include "SLevelOfDetailBranchNode.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "DialoguesEditor"

class SDialoguePin : public SGraphPinAI
{
public:
	SLATE_BEGIN_ARGS(SDialoguePin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
	/** @return The color that we should use to draw this pin */
	virtual FSlateColor GetPinColor() const override;
};

void SDialoguePin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	SGraphPinAI::Construct(SGraphPinAI::FArguments(), InPin);
}

FSlateColor SDialoguePin::GetPinColor() const
{
	return
		bIsDiffHighlighted ? DialogueEditorColors::Pin::Diff :
		IsHovered() ? DialogueEditorColors::Pin::Hover :
		(GraphPinObj->PinType.PinCategory == UDialogueGraphTypes::PinCategory_SingleComposite) ? DialogueEditorColors::Pin::CompositeOnly :
		(GraphPinObj->PinType.PinCategory == UDialogueGraphTypes::PinCategory_SingleStep) ? DialogueEditorColors::Pin::StepOnly :
		(GraphPinObj->PinType.PinCategory == UDialogueGraphTypes::PinCategory_SingleNode) ? DialogueEditorColors::Pin::SingleNode :
		DialogueEditorColors::Pin::Default;
}

void SGraphNode_Dialogues::Construct(const FArguments& InArgs, UDialoguesGraphNode* InNode)
{
	DebuggerStateDuration = 0.0f;
	DebuggerStateCounter = INDEX_NONE;
	bSuppressDebuggerTriggers = false;

	SGraphNodeAI::Construct(SGraphNodeAI::FArguments(), InNode);
}

void SGraphNode_Dialogues::UpdateGraphNode()
{
	bDragMarkerVisible = false;
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	SubNodes.Reset();
	OutputPinBox.Reset();

	ChildNodeInfo.Reset();
	ChildNodeInfo.Add(EDialogueGraphSubNodeWidgetLocation::Above).ChildNodeBox = SNew(SVerticalBox);
	ChildNodeInfo.Add(EDialogueGraphSubNodeWidgetLocation::Below).ChildNodeBox = SNew(SVerticalBox);

	UDialoguesGraphNode* DialogueNode = Cast<UDialoguesGraphNode>(GraphNode);

	if (DialogueNode)
	{
		for (UAIGraphNode* TestNode : DialogueNode->SubNodes)
		{
			if (UDialoguesGraphNode_Requirement* RequirementNode = Cast<UDialoguesGraphNode_Requirement>(TestNode))
			{
				TSharedPtr<SGraphNode> NewNode = FNodeFactory::CreateNodeWidget(RequirementNode);
				AddSubNodeWidget(NewNode, EDialogueGraphSubNodeWidgetLocation::Above);
			}
		}
	}

	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<STextBlock> DescriptionText;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	TWeakPtr<SNodeTitle> WeakNodeTitle = NodeTitle;
	auto GetNodeTitlePlaceholderWidth = [WeakNodeTitle]() -> FOptionalSize
		{
			TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
			const float DesiredWidth = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().X : 0.0f;
			return FMath::Max(75.0f, DesiredWidth);
		};
	auto GetNodeTitlePlaceholderHeight = [WeakNodeTitle]() -> FOptionalSize
		{
			TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
			const float DesiredHeight = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().Y : 0.0f;
			return FMath::Max(22.0f, DesiredHeight);
		};

	const bool bIsEmbeddedNode = DialogueNode && DialogueNode->IsSubNode();
	const FMargin NodePadding = bIsEmbeddedNode ? FMargin(2.0f) : FMargin(8.0f);

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FPropertyRowGeneratorArgs Args;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;
	//Args.NotifyHook = this;
	PropertyRowGenerator = PropertyEditorModule.CreatePropertyRowGenerator(Args);

	if (UDialoguesGraphNode* DialogueGraphNode = Cast<UDialoguesGraphNode>(GraphNode))
	{
		if (DialogueGraphNode->NodeInstance)
		{
			PropertyRowGenerator->SetObjects({ DialogueGraphNode->NodeInstance });
		}
	}

	PropertyRowGenerator->OnRowsRefreshed().AddSP(this, &SGraphNode_Dialogues::PropertyRowsRefreshed);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
				.Padding(0.0f)
				.BorderBackgroundColor(this, &SGraphNode_Dialogues::GetBorderBackgroundColor)
				.OnMouseButtonDown(this, &SGraphNode_Dialogues::OnMouseDown)
				[
					SNew(SOverlay)

						// Pins and node details
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SVerticalBox)

								// INPUT PIN AREA
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SBox)
										.MinDesiredHeight(NodePadding.Top)
										[
											SAssignNew(LeftNodeBox, SVerticalBox)
										]
								]

								// STATE NAME AREA
								+ SVerticalBox::Slot()
								.Padding(FMargin(NodePadding.Left, 0.0f, NodePadding.Right, 0.0f))
								[
									SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											ChildNodeInfo[EDialogueGraphSubNodeWidgetLocation::Above].ChildNodeBox.ToSharedRef()
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SAssignNew(NodeBody, SBorder)
												.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
												.BorderBackgroundColor(this, &SGraphNode_Dialogues::GetBackgroundColor)
												.HAlign(HAlign_Fill)
												.VAlign(VAlign_Center)
												.Visibility(EVisibility::SelfHitTestInvisible)
												[
													SNew(SOverlay)
														+ SOverlay::Slot()
														.HAlign(HAlign_Fill)
														.VAlign(VAlign_Fill)
														[
															SNew(SVerticalBox)
																+ SVerticalBox::Slot()
																.AutoHeight()
																[
																	SNew(SHorizontalBox)
																		+ SHorizontalBox::Slot()
																		.AutoWidth()
																		[
																			// POPUP ERROR MESSAGE
																			SAssignNew(ErrorText, SErrorText)
																				.BackgroundColor(this, &SGraphNode_Dialogues::GetErrorColor)
																				.ToolTipText(this, &SGraphNode_Dialogues::GetErrorMsgToolTip)
																		]

																		+ SHorizontalBox::Slot()
																		.AutoWidth()
																		[
																			SNew(SLevelOfDetailBranchNode)
																				.UseLowDetailSlot(this, &SGraphNode_Dialogues::UseLowDetailNodeTitles)
																				.LowDetail()
																				[
																					SNew(SBox)
																						.WidthOverride_Lambda(GetNodeTitlePlaceholderWidth)
																						.HeightOverride_Lambda(GetNodeTitlePlaceholderHeight)
																				]
																				.HighDetail()
																				[
																					SNew(SHorizontalBox)
																						+ SHorizontalBox::Slot()
																						.AutoWidth()
																						.VAlign(VAlign_Center)
																						[
																							SNew(SImage)
																								.Image(this, &SGraphNode_Dialogues::GetNameIcon)
																						]
																						+ SHorizontalBox::Slot()
																						.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
																						[
																							SNew(SVerticalBox)
																								+ SVerticalBox::Slot()
																								.AutoHeight()
																								[
																									SAssignNew(InlineEditableText, SInlineEditableTextBlock)
																										.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
																										.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
																										.OnVerifyTextChanged(this, &SGraphNode_Dialogues::OnVerifyNameTextChanged)
																										.OnTextCommitted(this, &SGraphNode_Dialogues::OnNameTextCommited)
																										.IsReadOnly(this, &SGraphNode_Dialogues::IsNameReadOnly)
																										.IsSelected(this, &SGraphNode_Dialogues::IsSelectedExclusively)
																								]
																								+ SVerticalBox::Slot()
																								.AutoHeight()
																								[
																									NodeTitle.ToSharedRef()
																								]
																						]
																				]
																		]
																]

																+ SVerticalBox::Slot()
																.AutoHeight()
																[
																	// DESCRIPTION MESSAGE
																	SAssignNew(DescriptionText, STextBlock)
																		.Visibility(this, &SGraphNode_Dialogues::GetDescriptionVisibility)
																		.Text(this, &SGraphNode_Dialogues::GetDescription)
																]

																+ SVerticalBox::Slot()
																.Expose(PropertyDetailsSlot)
																.AutoHeight()
																[
																	SNullWidget::NullWidget
																]
														]
												]
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											ChildNodeInfo[EDialogueGraphSubNodeWidgetLocation::Below].ChildNodeBox.ToSharedRef()
										]
								]

								// OUTPUT PIN AREA
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SBox)
										.MinDesiredHeight(NodePadding.Bottom)
										[
											SAssignNew(RightNodeBox, SVerticalBox)
												+ SVerticalBox::Slot()
												.HAlign(HAlign_Fill)
												.VAlign(VAlign_Fill)
												.Padding(20.0f, 0.0f)
												.FillHeight(1.0f)
												[
													SAssignNew(OutputPinBox, SHorizontalBox)
												]
										]
								]
						]

						// Drag marker overlay
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Top)
						[
							SNew(SBorder)
								.BorderBackgroundColor(DialogueEditorColors::Action::DragMarker)
								.ColorAndOpacity(DialogueEditorColors::Action::DragMarker)
								.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
								.Visibility(this, &SGraphNode_Dialogues::GetDragOverMarkerVisibility)
								[
									SNew(SBox)
										.HeightOverride(4)
								]
						]

						// Blueprint indicator overlay
						+ SOverlay::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						[
							SNew(SImage)
								.Image(FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Blueprint")))
								.Visibility(this, &SGraphNode_Dialogues::GetBlueprintIconVisibility)
						]
				]
		];
	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

	ErrorReporting = ErrorText;
	//ErrorReporting->SetError(TEXT("Testerror"));
	UpdateErrorInfo();

	PropertyRowsRefreshed();

	CreatePinWidgets();
}

void SGraphNode_Dialogues::CreatePinWidgets()
{
	UDialoguesGraphNode* StateNode = CastChecked<UDialoguesGraphNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SDialoguePin, MyPin)
				.ToolTipText(this, &SGraphNode_Dialogues::GetPinTooltip, MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SGraphNode_Dialogues::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility(TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced));
	}

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		LeftNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			.Padding(20.0f, 0.0f)
			[
				PinToAdd
			];
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		const bool bIsSingleStepPin = PinObj && (PinObj->PinType.PinCategory == UDialogueGraphTypes::PinCategory_SingleStep);
		if (bIsSingleStepPin)
		{
			OutputPinBox->AddSlot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.FillWidth(0.4f)
				.Padding(0, 0, 20.0f, 0)
				[
					PinToAdd
				];
		}
		else
		{
			OutputPinBox->AddSlot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.FillWidth(1.0f)
				[
					PinToAdd
				];
		}
		OutputPins.Add(PinToAdd);
	}
}

TSharedPtr<SToolTip> SGraphNode_Dialogues::GetComplexTooltip()
{
	return IDocumentation::Get()->CreateToolTip(TAttribute<FText>(this, &SGraphNode::GetNodeTooltip), NULL, GraphNode->GetDocumentationLink(), GraphNode->GetDocumentationExcerptName());
}

TArray<FOverlayWidgetInfo> SGraphNode_Dialogues::GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const
{
	TArray<FOverlayWidgetInfo> Widgets;

	check(NodeBody.IsValid());

	FVector2D Origin(0.0f, 0.0f);

	// build overlays for sub-nodes (above)
	for (const TSharedPtr<SGraphNode>& ChildWidget : ChildNodeInfo.FindChecked(EDialogueGraphSubNodeWidgetLocation::Above).ChildNodeWidgets)
	{
		TArray<FOverlayWidgetInfo> OverlayWidgets = ChildWidget->GetOverlayWidgets(bSelected, WidgetSize);
		for (auto& OverlayWidget : OverlayWidgets)
		{
			OverlayWidget.OverlayOffset.Y += Origin.Y;
		}
		Widgets.Append(OverlayWidgets);
		Origin.Y += ChildWidget->GetDesiredSize().Y;
	}

	Origin.Y += NodeBody->GetDesiredSize().Y;

	// build overlays for sub-nodes (below)
	for (const TSharedPtr<SGraphNode>& ChildWidget : ChildNodeInfo.FindChecked(EDialogueGraphSubNodeWidgetLocation::Below).ChildNodeWidgets)
	{
		TArray<FOverlayWidgetInfo> OverlayWidgets = ChildWidget->GetOverlayWidgets(bSelected, WidgetSize);
		for (auto& OverlayWidget : OverlayWidgets)
		{
			OverlayWidget.OverlayOffset.Y += Origin.Y;
		}
		Widgets.Append(OverlayWidgets);
		Origin.Y += ChildWidget->GetDesiredSize().Y;
	}

	return Widgets;
}

TSharedRef<SGraphNode> SGraphNode_Dialogues::GetNodeUnderMouse(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TSharedPtr<SGraphNode> SubNode = GetSubNodeUnderCursor(MyGeometry, MouseEvent);
	return SubNode.IsValid() ? SubNode.ToSharedRef() : StaticCastSharedRef<SGraphNode>(AsShared());
}

void SGraphNode_Dialogues::AddSubNodeWidget(TSharedPtr<SGraphNode> NewSubNodeWidget, EDialogueGraphSubNodeWidgetLocation Location)
{
	/*if (OwnerGraphPanelPtr.IsValid())
	{
		NewSubNodeWidget->SetOwner(OwnerGraphPanelPtr.Pin().ToSharedRef());
		OwnerGraphPanelPtr.Pin()->AttachGraphEvents(NewSubNodeWidget);
	}*/
	NewSubNodeWidget->UpdateGraphNode();


	FSubNodeWidgetInfo& SubNodeAreaInfo = ChildNodeInfo.FindChecked(Location);

	SubNodeAreaInfo.ChildNodeBox->AddSlot().AutoHeight()
		[
			NewSubNodeWidget.ToSharedRef()
		];

	SubNodeAreaInfo.ChildNodeWidgets.Add(NewSubNodeWidget);

	AddSubNode(NewSubNodeWidget);
}

void SGraphNode_Dialogues::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SGraphNode::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	UDialoguesGraphNode* MyNode = Cast<UDialoguesGraphNode>(GraphNode);

	//@TODO: CONVERSATION: DEBUGGER
// 	if (MyNode && MyNode->DebuggerUpdateCounter != DebuggerStateCounter)
// 	{
// 		DebuggerStateCounter = MyNode->DebuggerUpdateCounter;
// 		DebuggerStateDuration = 0.0f;
// 		bSuppressDebuggerColor = false;
// 		bSuppressDebuggerTriggers = false;
// 	}

	DebuggerStateDuration += InDeltaTime;
}

FSlateColor SGraphNode_Dialogues::GetBorderBackgroundColor() const
{
	UDialoguesGraphNode* DialoguesGraphNode = Cast<UDialoguesGraphNode>(GraphNode);

	const bool bSelectedNode = GetOwnerPanel()->SelectionManager.SelectedNodes.Contains(GraphNode);

	if (bSelectedNode)
	{
		return DialogueEditorColors::NodeBorder::Selected;
	}

	//@TODO: CONVERSATION: Search highlighting
// 		if (DialogueGraphNode->bHighlightInSearchTree)
// 		{
// 			return DialogueEditorColors::NodeBorder::QuickFind;
// 		}

	if (!CanReachNode())
		return DialogueEditorColors::NodeBorder::Disconnected;

	return DialogueEditorColors::NodeBorder::Inactive;
}

FSlateColor SGraphNode_Dialogues::GetBackgroundColor() const
{
	UDialoguesGraphNode* DialogueNode = CastChecked<UDialoguesGraphNode>(GraphNode);

	FLinearColor NodeColor = DialogueNode->GetNodeBodyTintColor();

	if (DialogueNode->HasErrors())
	{
		NodeColor = DialogueEditorColors::NodeBody::Error;
	}

	return NodeColor;
}

const FSlateBrush* SGraphNode_Dialogues::GetNameIcon() const
{
	UDialoguesGraphNode* DialogueGraphNode = Cast<UDialoguesGraphNode>(GraphNode);
	return DialogueGraphNode != nullptr ? FDialoguesStyle::Get().GetBrush(DialogueGraphNode->GetNameIcon()) : FDialoguesStyle::Get().GetBrush(TEXT("Dialogues.Graph.Icon"));
}

EVisibility SGraphNode_Dialogues::GetBlueprintIconVisibility() const
{
	UDialoguesGraphNode* DialogueGraphNode = Cast<UDialoguesGraphNode>(GraphNode);
	const bool bCanShowIcon = (DialogueGraphNode != nullptr && DialogueGraphNode->UsesBlueprint());

	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (bCanShowIcon && (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail)) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SGraphNode_Dialogues::GetPinTooltip(UEdGraphPin* GraphPinObj) const
{
	FText HoverText = FText::GetEmpty();

	check(GraphPinObj != nullptr);
	UEdGraphNode* OwningGraphNode = GraphPinObj->GetOwningNode();
	if (OwningGraphNode != nullptr)
	{
		FString HoverStr;
		OwningGraphNode->GetPinHoverText(*GraphPinObj, /*out*/HoverStr);
		if (!HoverStr.IsEmpty())
		{
			HoverText = FText::FromString(HoverStr);
		}
	}

	return HoverText;
}

void SGraphNode_Dialogues::PropertyRowsRefreshed()
{
	if (UDialoguesGraphNode* DialogueGraphNode = Cast<UDialoguesGraphNode>(GraphNode))
	{
		if (UDialogueNode* DialogueNode = Cast<UDialogueNode>(DialogueGraphNode->NodeInstance))
		{
			if (!DialogueNode->ShowPropertyEditors())
			{
				return;
			}
		}
	}

	TSharedPtr<SWidget> TimeWidget = nullptr;
	TSharedPtr<SWidget> ValueWidget = nullptr;

	TSharedRef<SGridPanel> GridPanel =
		SNew(SGridPanel)
		.FillColumn(0, 0.0f)
		.FillColumn(1, 1.0f);

	static const FName NAME_ExposeOnSpawn(TEXT("ExposeOnSpawn"));

	int32 GridRow = 0;
	for (TSharedRef<IDetailTreeNode> RootNode : PropertyRowGenerator->GetRootTreeNodes())
	{
		TArray<TSharedRef<IDetailTreeNode>> ChildNodes;
		RootNode->GetChildren(ChildNodes);

		for (TSharedRef<IDetailTreeNode> ChildNode : ChildNodes)
		{
			if (ChildNode->GetNodeType() == EDetailNodeType::Category)
			{
				continue;
			}

			TSharedPtr<IPropertyHandle> ChildNodePH = ChildNode->CreatePropertyHandle();

			if (!ChildNodePH || !ChildNodePH->HasMetaData(NAME_ExposeOnSpawn))
			{
				continue;
			}

			FNodeWidgets NodeWidgets = ChildNode->CreateNodeWidgets();
			if (NodeWidgets.NameWidget && NodeWidgets.ValueWidget)
			{
				GridPanel->AddSlot(0, GridRow)
					[
						NodeWidgets.NameWidget.ToSharedRef()
					];

				GridPanel->AddSlot(1, GridRow)
					[
						NodeWidgets.ValueWidget.ToSharedRef()
					];

				GridRow++;
			}
		}
	}

	(*PropertyDetailsSlot)
		[
			GridPanel
		];
}

bool SGraphNode_Dialogues::CanReachNode() const
{
	UDialoguesGraphNode* DialogueGraphNode = Cast<UDialoguesGraphNode>(GraphNode);
	if (UDialogueNode_Step* StepNode = Cast<UDialogueNode_Step>(DialogueGraphNode->NodeInstance))
		return StepNode->bCanBeReached;

	return false;
}

#undef LOCTEXT_NAMESPACE