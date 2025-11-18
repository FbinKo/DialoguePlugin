#include "FindInDialogues.h"
#include "DialoguesEditor.h"
#include "DialoguesGraphNode.h"
#include "Widgets/Input/SSearchBox.h"

#define LOCTEXT_NAMESPACE "FindInDialogues"

FFindInDialoguesResult::FFindInDialoguesResult(const FString& InValue)
	: Value(InValue), GraphNode(NULL)
{
}

FFindInDialoguesResult::FFindInDialoguesResult(const FString& InValue, TSharedPtr<FFindInDialoguesResult>& InParent, UEdGraphNode* InNode)
	: Value(InValue), GraphNode(InNode), Parent(InParent)
{
}

FReply FFindInDialoguesResult::OnClick(TWeakPtr<class FDialoguesEditor> DialoguesEditor, TSharedPtr<FFindInDialoguesResult> Root)
{
	if (DialoguesEditor.IsValid() && GraphNode.IsValid())
	{
		if (Parent.IsValid() && Parent.HasSameObject(Root.Get()))
		{
			DialoguesEditor.Pin()->JumpToNode(GraphNode.Get());
		}
		else
		{
			DialoguesEditor.Pin()->JumpToNode(Parent.Pin()->GraphNode.Get());
		}
	}

	return FReply::Handled();
}

TSharedRef<SWidget> FFindInDialoguesResult::CreateIcon() const
{
	FSlateColor IconColor = FSlateColor::UseForeground();
	const FSlateBrush* Brush = NULL;

	if (GraphNode.IsValid())
	{
		/*if (Cast<UDialoguesGraphNode_Service>(GraphNode.Get()))
		{
			Brush = FAppStyle::GetBrush(TEXT("GraphEditor.PinIcon"));
		}
		else if (Cast<UDialoguesGraphNode_Decorator>(GraphNode.Get()))
		{
			Brush = FAppStyle::GetBrush(TEXT("GraphEditor.RefPinIcon"));
		}
		else
		{
			Brush = FAppStyle::GetBrush(TEXT("GraphEditor.FIB_Event"));
		}*/
	}

	return SNew(SImage)
		.Image(Brush)
		.ColorAndOpacity(IconColor);
}

FString FFindInDialoguesResult::GetCommentText() const
{
	if (GraphNode.IsValid())
	{
		return GraphNode->NodeComment;
	}

	return FString();
}

FString FFindInDialoguesResult::GetNodeTypeText() const
{
	if (GraphNode.IsValid())
	{
		FString NodeClassName = GraphNode->GetClass()->GetName();
		int32 Pos = NodeClassName.Find("_");
		if (Pos == INDEX_NONE)
		{
			return NodeClassName;
		}
		else
		{
			return NodeClassName.RightChop(Pos + 1);
		}
	}

	return FString();
}

void FFindInDialoguesResult::SetNodeHighlight(bool bHighlight)
{
	if (GraphNode.IsValid())
	{
		UDialoguesGraphNode* BTNode = Cast<UDialoguesGraphNode>(GraphNode.Get());
		if (BTNode)
		{
			//BTNode->bHighlightInSearchTree = bHighlight;
		}
	}
}

void SFindInDialogues::Construct(const FArguments& InArgs, TSharedPtr<class FDialoguesEditor> InDialoguesEditor)
{
	DialoguesEditorPtr = InDialoguesEditor;

	this->ChildSlot
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						[
							SAssignNew(SearchTextField, SSearchBox)
								.HintText(LOCTEXT("DialoguesSearchHint", "Enter text to find nodes..."))
								.OnTextChanged(this, &SFindInDialogues::OnSearchTextChanged)
								.OnTextCommitted(this, &SFindInDialogues::OnSearchTextCommitted)
						]
				]
			+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("Menu.Background"))
						[
							SAssignNew(TreeView, STreeViewType)
								.TreeItemsSource(&ItemsFound)
								.OnGenerateRow(this, &SFindInDialogues::OnGenerateRow)
								.OnGetChildren(this, &SFindInDialogues::OnGetChildren)
								.OnSelectionChanged(this, &SFindInDialogues::OnTreeSelectionChanged)
								.SelectionMode(ESelectionMode::Multi)
						]
				]
		];
}

void SFindInDialogues::FocusForUse()
{
	// NOTE: Careful, GeneratePathToWidget can be reentrant in that it can call visibility delegates and such
	FWidgetPath FilterTextBoxWidgetPath;
	FSlateApplication::Get().GeneratePathToWidgetUnchecked(SearchTextField.ToSharedRef(), FilterTextBoxWidgetPath);

	// Set keyboard focus directly
	FSlateApplication::Get().SetKeyboardFocus(FilterTextBoxWidgetPath, EFocusCause::SetDirectly);
}

void SFindInDialogues::OnSearchTextChanged(const FText& Text)
{
	SearchValue = Text.ToString();

	InitiateSearch();
}

void SFindInDialogues::OnSearchTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	OnSearchTextChanged(Text);
}

void SFindInDialogues::OnGetChildren(FSearchResult InItem, TArray<FSearchResult>& OutChildren)
{
	OutChildren += InItem->Children;
}

void SFindInDialogues::OnTreeSelectionChanged(FSearchResult Item, ESelectInfo::Type SelectInfo)
{
	if (Item.IsValid())
	{
		Item->OnClick(DialoguesEditorPtr, RootSearchResult);
	}
}

TSharedRef<ITableRow> SFindInDialogues::OnGenerateRow(FSearchResult InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow< TSharedPtr<FFindInDialoguesResult> >, OwnerTable)
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(SBox)
						.WidthOverride(450.0f)
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									InItem->CreateIcon()
								]
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
								.AutoWidth()
								.Padding(2, 0)
								[
									SNew(STextBlock)
										.Text(FText::FromString(InItem->Value))
										.HighlightText(HighlightText)
								]
						]
				]
			+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString(InItem->GetNodeTypeText()))
						.HighlightText(HighlightText)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString(InItem->GetCommentText()))
						.ColorAndOpacity(FLinearColor::Yellow)
						.HighlightText(HighlightText)
				]
		];
}

void SFindInDialogues::InitiateSearch()
{
	TArray<FString> Tokens;
	SearchValue.ParseIntoArray(Tokens, TEXT(" "), true);

	for (auto It(ItemsFound.CreateIterator()); It; ++It)
	{
		(*It).Get()->SetNodeHighlight(false); // need to reset highlight
		TreeView->SetItemExpansion(*It, false);
	}
	ItemsFound.Empty();
	if (Tokens.Num() > 0)
	{
		HighlightText = FText::FromString(SearchValue);
		MatchTokens(Tokens);
	}

	// Insert a fake result to inform user if none found
	if (ItemsFound.Num() == 0)
	{
		ItemsFound.Add(FSearchResult(new FFindInDialoguesResult(LOCTEXT("DialoguesSearchNoResults", "No Results found").ToString())));
	}

	TreeView->RequestTreeRefresh();

	for (auto It(ItemsFound.CreateIterator()); It; ++It)
	{
		TreeView->SetItemExpansion(*It, true);
	}
}

void SFindInDialogues::MatchTokens(const TArray<FString>& Tokens)
{
	RootSearchResult.Reset();

	TWeakPtr<SGraphEditor> FocusedGraphEditor = DialoguesEditorPtr.Pin()->GetFocusedGraphPtr();
	UEdGraph* Graph = NULL;
	if (FocusedGraphEditor.IsValid())
	{
		Graph = FocusedGraphEditor.Pin()->GetCurrentGraph();
	}

	if (Graph == NULL)
	{
		return;
	}

	RootSearchResult = FSearchResult(new FFindInDialoguesResult(FString("DialoguesRoot")));

	for (auto It(Graph->Nodes.CreateConstIterator()); It; ++It)
	{
		UEdGraphNode* Node = *It;

		const FString NodeName = Node->GetNodeTitle(ENodeTitleType::ListView).ToString();
		FSearchResult NodeResult(new FFindInDialoguesResult(NodeName, RootSearchResult, Node));

		FString NodeSearchString = NodeName + Node->GetClass()->GetName() + Node->NodeComment;
		NodeSearchString = NodeSearchString.Replace(TEXT(" "), TEXT(""));

		bool bNodeMatchesSearch = StringMatchesSearchTokens(Tokens, NodeSearchString);

		UDialoguesGraphNode* BTNode = Cast<UDialoguesGraphNode>(Node);
		if (BTNode)
		{
			for (auto SubNodeIt(BTNode->SubNodes.CreateConstIterator()); SubNodeIt; ++SubNodeIt)
			{
				UDialoguesGraphNode* SubNode = Cast<UDialoguesGraphNode>(*SubNodeIt);
				MatchTokensInChild(Tokens, SubNode, NodeResult);
			}
		}

		if ((NodeResult->Children.Num() > 0) || bNodeMatchesSearch)
		{
			NodeResult->SetNodeHighlight(true);
			ItemsFound.Add(NodeResult);
		}
	}
}

void SFindInDialogues::MatchTokensInChild(const TArray<FString>& Tokens, UDialoguesGraphNode* Child, FSearchResult ParentNode)
{
	if (Child == NULL)
	{
		return;
	}

	FString ChildName = Child->GetNodeTitle(ENodeTitleType::ListView).ToString();
	FString ChildSearchString = ChildName + Child->GetClass()->GetName() + Child->NodeComment;
	ChildSearchString = ChildSearchString.Replace(TEXT(" "), TEXT(""));
	if (StringMatchesSearchTokens(Tokens, ChildSearchString))
	{
		FSearchResult DecoratorResult(new FFindInDialoguesResult(ChildName, ParentNode, Child));
		ParentNode->Children.Add(DecoratorResult);
	}
}

bool SFindInDialogues::StringMatchesSearchTokens(const TArray<FString>& Tokens, const FString& ComparisonString)
{
	bool bFoundAllTokens = true;

	//search the entry for each token, it must have all of them to pass
	for (auto TokIT(Tokens.CreateConstIterator()); TokIT; ++TokIT)
	{
		const FString& Token = *TokIT;
		if (!ComparisonString.Contains(Token))
		{
			bFoundAllTokens = false;
			break;
		}
	}
	return bFoundAllTokens;
}

#undef LOCTEXT_NAMESPACE
