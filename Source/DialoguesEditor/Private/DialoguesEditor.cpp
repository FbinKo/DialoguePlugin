#include "DialoguesEditor.h"
#include "Dialogues.h"
#include "DialoguesEditorCommands.h"
#include "DialoguesEditorModule.h"
#include "DialoguesEditorToolbar.h"
#include "DialoguesEditorTabFactories.h"
#include "DialoguesEditorTabs.h"
#include "DialoguesEditorModes.h"
#include "FindInDialogues.h"
#include "DialoguesGraph.h"
#include "DialoguesGraphNode.h"
#include "DialoguesDebugger.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "GraphEditorActions.h"
#include "Framework/Docking/TabManager.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "ClassViewerFilter.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "DialoguesEditor"

const FName FDialoguesEditor::DialoguesMode(TEXT("Dialogues"));

class FNewNodeClassFilter : public IClassViewerFilter
{
public:
	FNewNodeClassFilter(UClass* InBaseClass)
		: BaseClass(InBaseClass)
	{
	}

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		if (InClass != nullptr)
		{
			return InClass->IsChildOf(BaseClass);
		}
		return false;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		return InUnloadedClassData->IsChildOf(BaseClass);
	}

private:
	UClass* BaseClass;
};

FDialoguesEditor::FDialoguesEditor()
	: IDialoguesEditor()
{
	Dialogues = nullptr;
}

FDialoguesEditor::~FDialoguesEditor()
{
	Debugger.Reset();
}

void FDialoguesEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	DocumentManager->SetTabManager(InTabManager);

	FWorkflowCentricApplication::RegisterTabSpawners(InTabManager);
}

void FDialoguesEditor::InitDialoguesEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UObject* InObject)
{
	UDialogues* dialoguesToEdit = Cast<UDialogues>(InObject);
	if (dialoguesToEdit) Dialogues = dialoguesToEdit;

	if (!ToolbarBuilder.IsValid()) ToolbarBuilder = MakeShareable(new FDialoguesEditorToolbar(SharedThis(this)));

	if (!DocumentManager.IsValid())
	{
		DocumentManager = MakeShareable(new FDocumentTracker);
		DocumentManager->Initialize(SharedThis(this));

		//register document factories
		{
			TSharedRef<FDocumentTabFactory> GraphEditorFactory = MakeShareable(new FDialoguesTabFactory(SharedThis(this),
				FDialoguesTabFactory::FOnCreateGraphEditorWidget::CreateSP(this, &FDialoguesEditor::CreateGraphEditorWidget)));

			//store reference to factory so all spawned tabs can be found later
			GraphEditorTabFactoryPtr = GraphEditorFactory;
			DocumentManager->RegisterDocumentFactory(GraphEditorFactory);
		}
	}

	TArray<UObject*> ObjectBeingEdited;
	ObjectBeingEdited.Add(InObject);

	FGraphEditorCommands::Register();
	FDialoguesCommonCommands::Register();
	//FBTDebuggerCommands::Register();

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	InitAssetEditor(Mode, InitToolkitHost, FDialoguesEditorModule::DialoguesEditorAppIdentifier, FTabManager::FLayout::NullLayout,
		bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectBeingEdited);

	BindCommonCommands();
	ExtendMenu();
	CreateInternalWidgets();

	Debugger = MakeShareable(new FDialoguesDebugger);
	Debugger->Setup(Dialogues, SharedThis(this));
	BindDebuggerToolbarCommands();

	FDialoguesEditorModule& DialoguesEditorModule = FModuleManager::LoadModuleChecked<FDialoguesEditorModule>("DialoguesEditor");
	AddMenuExtender(DialoguesEditorModule.GetMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));

	AddApplicationMode(DialoguesMode, MakeShareable(new FDialoguesEditorApplicationMode(SharedThis(this))));
	//AddApplicationMode...

	SetCurrentMode(DialoguesMode);
}

FName FDialoguesEditor::GetToolkitFName() const
{
	return FName("DialoguesEditor");
}

FText FDialoguesEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Dialogues Editor");
}

FString FDialoguesEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Dialogues Editor ").ToString();
}

FLinearColor FDialoguesEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Blue;
}

FText FDialoguesEditor::GetToolkitName() const
{
	return FAssetEditorToolkit::GetLabelForObject(GetDialogues());
}

FText FDialoguesEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(GetDialogues());
}

void FDialoguesEditor::DoubleClickNode(UEdGraphNode* Node)
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = UpdateGraphEdPtr.Pin();
	if (CurrentGraphEditor.IsValid())
	{
		CurrentGraphEditor->ClearSelectionSet();
		CurrentGraphEditor->SetNodeSelection(Node, true);
	}

	JumpToNode(Node);
	OnNodeDoubleClicked(Node);
}

void FDialoguesEditor::FocusWindow(UObject* ObjectToFocusOn)
{
	FWorkflowCentricApplication::FocusWindow(ObjectToFocusOn);
}

void FDialoguesEditor::OnNodeDoubleClicked(UEdGraphNode* Node)
{
	UDialoguesGraphNode* MyNode = Cast<UDialoguesGraphNode>(Node);

	if (MyNode && MyNode->NodeInstance &&
		MyNode->NodeInstance->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UClass* NodeClass = MyNode->NodeInstance->GetClass();
		UPackage* Pkg = NodeClass->GetOuterUPackage();
		FString ClassName = NodeClass->GetName().LeftChop(2);
		UBlueprint* BlueprintOb = FindObject<UBlueprint>(Pkg, *ClassName);
		if (BlueprintOb)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(BlueprintOb);
		}
	}
}

void FDialoguesEditor::OnGraphEditorFocused(const TSharedRef<SGraphEditor>& InGraphEditor)
{
	UpdateGraphEdPtr = InGraphEditor;

	FGraphPanelSelectionSet CurrentSelection;
	CurrentSelection = InGraphEditor->GetSelectedNodes();
	OnSelectedNodesChanged(CurrentSelection);
}

void FDialoguesEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		static const FText TranscationTitle = FText::FromString(FString(TEXT("Rename Node")));
		const FScopedTransaction Transaction(TranscationTitle);
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

void FDialoguesEditor::SearchTree()
{
	TabManager->TryInvokeTab(FDialoguesEditorTabs::SearchID);
	FindResults->FocusForUse();
}

bool FDialoguesEditor::CanSearchTree() const
{
	return true;
}

void FDialoguesEditor::JumpToNode(const UEdGraphNode* Node)
{
	TSharedPtr<SDockTab> ActiveTab = DocumentManager->GetActiveTab();
	if (ActiveTab.IsValid())
	{
		TSharedPtr<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(ActiveTab->GetContent());
		if (GraphEditor.IsValid())
		{
			GraphEditor->JumpToNode(Node, false);
		}
	}
}

bool FDialoguesEditor::IsPropertyEditable() const
{
	/*if (FBehaviorTreeDebugger::IsPIESimulating() || bForceDisablePropertyEdit)
	{
		return false;
	}*/

	TSharedPtr<SGraphEditor> FocusedGraphEd = UpdateGraphEdPtr.Pin();
	return FocusedGraphEd.IsValid() && FocusedGraphEd->GetCurrentGraph() && FocusedGraphEd->GetCurrentGraph()->bEditable;
}

void FDialoguesEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	const TSharedPtr<SGraphEditor> FocusedGraphEd = UpdateGraphEdPtr.Pin();
	if (FocusedGraphEd.IsValid() && FocusedGraphEd->GetCurrentGraph())
	{
		FocusedGraphEd->GetCurrentGraph()->GetSchema()->ForceVisualizationCacheClear();
	}

	UDialoguesGraph* MyGraph = Cast<UDialoguesGraph>(Dialogues->Graph);
	MyGraph->UpdateAsset();
}

bool FDialoguesEditor::IsDebuggerReady() const
{
	//TODO
	return false; //Debugger.IsValid() && Debugger->IsDebuggerReady();
}

FGraphAppearanceInfo FDialoguesEditor::GetGraphAppearance() const
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "DIALOGUES");

	/*const int32 StepIdx = Debugger.IsValid() ? Debugger->GetShownStateIndex() : 0;
	if (Debugger.IsValid() && !Debugger->IsDebuggerRunning())
	{
		AppearanceInfo.PIENotifyText = LOCTEXT("InactiveLabel", "INACTIVE");
	}
	else if (StepIdx)
	{
		AppearanceInfo.PIENotifyText = FText::Format(LOCTEXT("StepsBackLabelFmt", "STEPS BACK: {0}"), FText::AsNumber(StepIdx));
	}
	else if (FDialoguesDebugger::IsPlaySessionPaused())
	{
		AppearanceInfo.PIENotifyText = LOCTEXT("PausedLabel", "PAUSED");
	}*/

	return AppearanceInfo;
}

bool FDialoguesEditor::InEditingMode(bool bGraphIsEditable) const
{
	return bGraphIsEditable;//&& FDialoguesDebugger::IsPIENotSimulating();
}

TWeakPtr<SGraphEditor> FDialoguesEditor::GetFocusedGraphPtr() const
{
	return UpdateGraphEdPtr;
}

bool FDialoguesEditor::CanAccessDialoguesMode() const
{
	return Dialogues != nullptr;
}

FText FDialoguesEditor::GetLocalizedMode(FName InMode)
{
	static TMap< FName, FText > LocModes;

	if (LocModes.Num() == 0)
	{
		LocModes.Add(DialoguesMode, LOCTEXT("DialoguesMode", "Dialogues"));
	}

	check(InMode != NAME_None);
	const FText* OutDesc = LocModes.Find(InMode);
	check(OutDesc);
	return *OutDesc;
}

UDialogues* FDialoguesEditor::GetDialogues() const
{
	return Dialogues;
}

TSharedRef<SWidget> FDialoguesEditor::SpawnProperties()
{
	return
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.HAlign(HAlign_Fill)
		[
			DetailsView.ToSharedRef()
		];
}

TSharedRef<SWidget> FDialoguesEditor::SpawnSearch()
{
	FindResults = SNew(SFindInDialogues, SharedThis(this));
	return FindResults.ToSharedRef();
}

void FDialoguesEditor::RegisterToolbarTab(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

void FDialoguesEditor::RestoreDialogues()
{
	UDialoguesGraph* MyGraph = Cast<UDialoguesGraph>(Dialogues->Graph);
	const bool bNewGraph = MyGraph == NULL;
	if (MyGraph == NULL)
	{
		const TSubclassOf<UEdGraphSchema> SchemaClass = GetDefault<UDialoguesGraph>(UDialoguesGraph::StaticClass())->Schema;
		check(SchemaClass);
		Dialogues->Graph = FBlueprintEditorUtils::CreateNewGraph(Dialogues, "Dialogues Graph", UDialoguesGraph::StaticClass(), SchemaClass);
		MyGraph = Cast<UDialoguesGraph>(Dialogues->Graph);

		// Initialize the behavior tree graph
		const UEdGraphSchema* Schema = MyGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*MyGraph);

		MyGraph->OnCreated();
	}
	else
	{
		MyGraph->OnLoaded();
	}

	MyGraph->Initialize();

	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(MyGraph);
	TSharedPtr<SDockTab> DocumentTab = DocumentManager->OpenDocument(Payload, bNewGraph ? FDocumentTracker::OpenNewDocument : FDocumentTracker::RestorePreviousDocument);

	if (Dialogues->LastEditedDocuments.Num() > 0)
	{
		TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(DocumentTab->GetContent());
		GraphEditor->SetViewLocation(Dialogues->LastEditedDocuments[0].SavedViewOffset, Dialogues->LastEditedDocuments[0].SavedZoomAmount);
	}

	const bool bIncreaseVersionNum = false;
	if (bNewGraph)
	{
		MyGraph->UpdateAsset(/*UDialoguesGraph::ClearDebuggerFlags | UDialoguesGraph::KeepRebuildCounter*/);
	}
	else
	{
		MyGraph->UpdateAsset(/*UDialoguesGraph::KeepRebuildCounter*/);
		//RefreshDebugger();
	}
}

void FDialoguesEditor::SaveEditedObjectState()
{
	// Clear currently edited documents
	Dialogues->LastEditedDocuments.Empty();

	// Ask all open documents to save their state, which will update LastEditedDocuments
	DocumentManager->SaveAllState();
}

bool FDialoguesEditor::CanCreateNewClasses() const
{
	return !IsDebuggerReady();
}

TSharedRef<SWidget> FDialoguesEditor::HandleCreateNewClassMenu(UClass* BaseClass) const
{
	FClassViewerInitializationOptions Options;
	Options.bShowUnloadedBlueprints = true;
	Options.ClassFilters.Add(MakeShareable(new FNewNodeClassFilter(BaseClass)));

	FOnClassPicked OnPicked(FOnClassPicked::CreateSP(this, &FDialoguesEditor::HandleNewNodeClassPicked));

	return FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer").CreateClassViewer(Options, OnPicked);
}

void FDialoguesEditor::HandleNewNodeClassPicked(UClass* InClass) const
{
	if (Dialogues)
	{
		const FString ClassName = FBlueprintEditorUtils::GetClassNameWithoutSuffix(InClass);

		FString PathName = Dialogues->GetOutermost()->GetPathName();
		PathName = FPaths::GetPath(PathName);

		// Now that we've generated some reasonable default locations/names for the package, allow the user to have the final say
		// before we create the package and initialize the blueprint inside of it.
		FSaveAssetDialogConfig SaveAssetDialogConfig;
		SaveAssetDialogConfig.DialogTitleOverride = LOCTEXT("SaveAssetDialogTitle", "Save Asset As");
		SaveAssetDialogConfig.DefaultPath = PathName;
		SaveAssetDialogConfig.DefaultAssetName = ClassName + TEXT("_New");
		SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;

		const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		const FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);
		if (!SaveObjectPath.IsEmpty())
		{
			const FString SavePackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
			const FString SavePackagePath = FPaths::GetPath(SavePackageName);
			const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);

			UPackage* Package = CreatePackage(*SavePackageName);
			if (ensure(Package))
			{
				// Create and init a new Blueprint
				if (UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(InClass, Package, FName(*SaveAssetName), BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass()))
				{
					GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewBP);

					// Notify the asset registry
					FAssetRegistryModule::AssetCreated(NewBP);

					// Mark the package dirty...
					Package->MarkPackageDirty();
				}
			}
		}
	}
}

void FDialoguesEditor::SaveAsset_Execute()
{
	if (Dialogues)
	{
		UDialoguesGraph* Graph = Cast<UDialoguesGraph>(Dialogues->Graph);
		if (Graph)
		{
			Graph->OnSave();
		}
	}
	// save it
	IDialoguesEditor::SaveAsset_Execute();
}

TSharedRef<class SGraphEditor> FDialoguesEditor::CreateGraphEditorWidget(UEdGraph* InGraph)
{
	check(InGraph != NULL);

	if (!GraphEditorCommands.IsValid())
	{
		CreateCommandList();

		// Debug actions
		/*GraphEditorCommands->MapAction(FGraphEditorCommands::Get().AddBreakpoint,
			FExecuteAction::CreateSP(this, &FDialoguesEditor::OnAddBreakpoint),
			FCanExecuteAction::CreateSP(this, &FDialoguesEditor::CanAddBreakpoint),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &FDialoguesEditor::CanAddBreakpoint)
		);

		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().RemoveBreakpoint,
			FExecuteAction::CreateSP(this, &FDialoguesEditor::OnRemoveBreakpoint),
			FCanExecuteAction::CreateSP(this, &FDialoguesEditor::CanRemoveBreakpoint),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &FDialoguesEditor::CanRemoveBreakpoint)
		);

		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().EnableBreakpoint,
			FExecuteAction::CreateSP(this, &FDialoguesEditor::OnEnableBreakpoint),
			FCanExecuteAction::CreateSP(this, &FDialoguesEditor::CanEnableBreakpoint),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &FDialoguesEditor::CanEnableBreakpoint)
		);

		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().DisableBreakpoint,
			FExecuteAction::CreateSP(this, &FDialoguesEditor::OnDisableBreakpoint),
			FCanExecuteAction::CreateSP(this, &FDialoguesEditor::CanDisableBreakpoint),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &FDialoguesEditor::CanDisableBreakpoint)
		);

		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().ToggleBreakpoint,
			FExecuteAction::CreateSP(this, &FDialoguesEditor::OnToggleBreakpoint),
			FCanExecuteAction::CreateSP(this, &FDialoguesEditor::CanToggleBreakpoint),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &FDialoguesEditor::CanToggleBreakpoint)
		);*/
	}

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FDialoguesEditor::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FDialoguesEditor::OnNodeDoubleClicked);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FDialoguesEditor::OnNodeTitleCommitted);

	// Make title bar
	TSharedRef<SWidget> TitleBarWidget =
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush(TEXT("Graph.TitleBackground")))
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.FillWidth(1.f)
				[
					SNew(STextBlock)
						.Text(LOCTEXT("DialoguesGraphLabel", "Dialogues"))
						.TextStyle(FAppStyle::Get(), TEXT("GraphBreadcrumbButtonText"))
				]
		];

	// Make full graph editor
	const bool bGraphIsEditable = InGraph->bEditable;
	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(this, &FDialoguesEditor::InEditingMode, bGraphIsEditable)
		.Appearance(this, &FDialoguesEditor::GetGraphAppearance)
		.TitleBar(TitleBarWidget)
		.GraphToEdit(InGraph)
		.GraphEvents(InEvents);
}

void FDialoguesEditor::CreateInternalWidgets()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(NULL);
	DetailsView->SetIsPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled::CreateSP(this, &FDialoguesEditor::IsPropertyEditable));
	DetailsView->OnFinishedChangingProperties().AddSP(this, &FDialoguesEditor::OnFinishedChangingProperties);
}

void FDialoguesEditor::ExtendMenu()
{
	struct Local
	{
		static void FillEditMenu(FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.BeginSection("EditSearch", LOCTEXT("EditMenu_SearchHeading", "Search"));
			{
				MenuBuilder.AddMenuEntry(FDialoguesCommonCommands::Get().SearchDialogues);
			}
			MenuBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);

	// Extend the Edit menu
	MenuExtender->AddMenuExtension(
		"EditHistory",
		EExtensionHook::After,
		GetToolkitCommands(),
		FMenuExtensionDelegate::CreateStatic(&Local::FillEditMenu));

	AddMenuExtender(MenuExtender);
}

void FDialoguesEditor::BindCommonCommands()
{
	ToolkitCommands->MapAction(FDialoguesCommonCommands::Get().SearchDialogues,
		FExecuteAction::CreateSP(this, &FDialoguesEditor::SearchTree),
		FCanExecuteAction::CreateSP(this, &FDialoguesEditor::CanSearchTree)
	);
}

void FDialoguesEditor::BindDebuggerToolbarCommands()
{
	//TODO
}

void FDialoguesEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	DialogueEditorUtils::FPropertySelectionInfo SelectionInfo;
	TArray<UObject*> Selection = DialogueEditorUtils::GetSelectionForPropertyEditor(NewSelection, SelectionInfo);

	UDialoguesGraph* MyGraph = Cast<UDialoguesGraph>(Dialogues->Graph);

	if (DetailsView.IsValid())
	{
		if (Selection.Num() == 0)
		{
			DetailsView->SetObject(Dialogues);
		}
		else if (Selection.Num() == 1)
		{
			DetailsView->SetObjects(Selection);
		}
		else
		{
			DetailsView->SetObject(nullptr);
		}
	}
}

#undef LOCTEXT_NAMESPACE

TArray<UObject*> DialogueEditorUtils::GetSelectionForPropertyEditor(const TSet<UObject*>& InSelection, FPropertySelectionInfo& OutSelectionInfo)
{
	TArray<UObject*> Selection;

	for (UObject* SelectionEntry : InSelection)
	{
		if (UDialoguesGraphNode* DialoguesNode = Cast<UDialoguesGraphNode>(SelectionEntry))
		{
			Selection.Add(DialoguesNode->NodeInstance);
			continue;
		}

		Selection.Add(SelectionEntry);
	}

	return Selection;
}
