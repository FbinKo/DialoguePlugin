#include "DialoguesEditorTabFactories.h"
#include "Dialogues.h"
#include "DialoguesEditor.h"
#include "DialoguesEditorTabs.h"

#define LOCTEXT_NAMESPACE "DialoguesEditorFactories"

FDialoguesTabFactory::FDialoguesTabFactory(TSharedPtr<class FDialoguesEditor> InDialoguesEditorPtr, FOnCreateGraphEditorWidget CreateGraphEditorWidgetCallback)
	: FDocumentTabFactoryForObjects<UEdGraph>(FDialoguesEditorTabs::GraphEditorID, InDialoguesEditorPtr)
	, DialoguesEditorPtr(InDialoguesEditorPtr)
	, OnCreateGraphEditorWidget(CreateGraphEditorWidgetCallback)
{
}

void FDialoguesTabFactory::OnTabActivated(TSharedPtr<SDockTab> Tab) const
{
	check(DialoguesEditorPtr.IsValid());
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	DialoguesEditorPtr.Pin()->OnGraphEditorFocused(GraphEditor);
}

void FDialoguesTabFactory::OnTabRefreshed(TSharedPtr<SDockTab> Tab) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	GraphEditor->NotifyGraphChanged();
}

TAttribute<FText> FDialoguesTabFactory::ConstructTabNameForObject(UEdGraph* DocumentID) const
{
	return TAttribute<FText>(FText::FromString(DocumentID->GetName()));
}

TSharedRef<SWidget> FDialoguesTabFactory::CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{
	return OnCreateGraphEditorWidget.Execute(DocumentID);
}

const FSlateBrush* FDialoguesTabFactory::GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{
	return FAppStyle::Get().GetBrush("DialoguesEditor.Dialogues");
}

void FDialoguesTabFactory::SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const
{
	check(DialoguesEditorPtr.IsValid());
	check(DialoguesEditorPtr.Pin()->GetDialogues());

	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());

	FVector2D ViewLocation;
	float ZoomAmount;
	GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);

	UEdGraph* Graph = FTabPayload_UObject::CastChecked<UEdGraph>(Payload);
	DialoguesEditorPtr.Pin()->GetDialogues()->LastEditedDocuments.Add(FEditedDocumentInfo(Graph, ViewLocation, ZoomAmount));
}

FDialoguesDetailsSummoner::FDialoguesDetailsSummoner(TSharedPtr<class FDialoguesEditor> InDialoguesEditorPtr)
	: FWorkflowTabFactory(FDialoguesEditorTabs::GraphDetailsID, InDialoguesEditorPtr)
	, DialoguesEditorPtr(InDialoguesEditorPtr)
{
	TabLabel = LOCTEXT("DialoguesDetailsLabel", "Details");
	TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DialoguesDetailsView", "Details");
	ViewMenuTooltip = LOCTEXT("DialoguesDetailsView_ToolTip", "Show the details view");
}

TSharedRef<SWidget> FDialoguesDetailsSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	check(DialoguesEditorPtr.IsValid());
	return DialoguesEditorPtr.Pin()->SpawnProperties();
}

FText FDialoguesDetailsSummoner::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DialoguesDetailsTabTooltip", "The dialogues details tab allows editing of the properties of dialogue nodes");
}

FDialoguesSearchSummoner::FDialoguesSearchSummoner(TSharedPtr<class FDialoguesEditor> InDialoguesEditorPtr)
	: FWorkflowTabFactory(FDialoguesEditorTabs::SearchID, InDialoguesEditorPtr)
	, DialoguesEditorPtr(InDialoguesEditorPtr)
{
	TabLabel = LOCTEXT("DialoguesSearchLabel", "Search");
	TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Kismet.Tabs.FindResults");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DialoguesSearchView", "Search");
	ViewMenuTooltip = LOCTEXT("DialoguesSearchView_ToolTip", "Show the behavior tree search tab");
}

TSharedRef<SWidget> FDialoguesSearchSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return DialoguesEditorPtr.Pin()->SpawnSearch();
}

FText FDialoguesSearchSummoner::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DialoguesSearchTabTooltip", "The dialogues search tab allows searching within dialogue nodes");
}

#undef LOCTEXT_NAMESPACE
