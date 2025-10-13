#pragma once

#include "AIGraphEditor.h"
#include "IDialoguesEditor.h"

class UDialoguesComponent;
class UDialogues;
class FDocumentTabFactory;
class FDocumentTracker;

namespace DialogueEditorUtils
{
	struct FPropertySelectionInfo
	{
		FPropertySelectionInfo()
		{
		}
	};

	/** Given a selection of nodes, return the instances that should be selected be selected for editing in the property panel */
	TArray<UObject*> GetSelectionForPropertyEditor(const TSet<UObject*>& InSelection, FPropertySelectionInfo& OutSelectionInfo);
}

class FDialoguesEditor : public IDialoguesEditor, public FAIGraphEditor, public FNotifyHook
{
public:
	FDialoguesEditor();
	/** Destructor */
	virtual ~FDialoguesEditor();

	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

	void InitDialoguesEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* InObject);

	//~ Begin IToolkit Interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	//~ End IToolkit Interface

	//~ Begin IDialoguesEditor Interface
	//virtual void InitializeDebuggerState(class FDialoguesDebugger* ParentDebugger) const override;
	virtual void DoubleClickNode(class UEdGraphNode* Node) override;
	virtual void FocusWindow(UObject* ObjectToFocusOn = NULL) override;
	//~ End IDialoguesEditor Interface

	// Delegates
	void OnNodeDoubleClicked(class UEdGraphNode* Node);
	void OnGraphEditorFocused(const TSharedRef<SGraphEditor>& InGraphEditor);
	void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);

	/*void OnEnableBreakpoint();
	bool CanEnableBreakpoint() const;
	void OnToggleBreakpoint();
	bool CanToggleBreakpoint() const;
	void OnDisableBreakpoint();
	bool CanDisableBreakpoint() const;
	void OnAddBreakpoint();
	bool CanAddBreakpoint() const;
	void OnRemoveBreakpoint();
	bool CanRemoveBreakpoint() const;*/

	void SearchTree();
	bool CanSearchTree() const;

	void JumpToNode(const UEdGraphNode* Node);

	bool IsPropertyEditable() const;
	void OnPackageSaved(const FString& PackageFileName, UPackage* Package, FObjectPostSaveContext ObjectSaveContext);
	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);

	void UpdateToolbar();
	bool IsDebuggerReady() const;

	///** Get whether the debugger is currently running and the PIE session is paused */
	//bool IsDebuggerPaused() const;

	///** Get whether we can edit the tree/blackboard with the debugger active */
	//bool CanEditWithDebuggerActive() const;

	//TSharedRef<class SWidget> OnGetDebuggerActorsMenu();
	//void OnDebuggerActorSelected(TWeakObjectPtr<UDialoguesComponent> InstanceToDebug);
	//FText GetDebuggerActorDesc() const;
	FGraphAppearanceInfo GetGraphAppearance() const;
	bool InEditingMode(bool bGraphIsEditable) const;

	/*void DebuggerSwitchAsset(UDialogues* NewAsset);
	void DebuggerUpdateGraph();

	EVisibility GetDebuggerDetailsVisibility() const;*/
	//EVisibility GetRangeLowerVisibility() const;
	//EVisibility GetRangeSelfVisibility() const;
	//EVisibility GetRootLevelNodeVisibility() const;

	TWeakPtr<SGraphEditor> GetFocusedGraphPtr() const;

	/** Check whether the behavior tree mode can be accessed (i.e whether we have a valid tree to edit) */
	bool CanAccessDialoguesMode() const;

	/** Check whether the blackboard mode can be accessed (i.e whether we have a valid blackboard to edit) */
	bool CanAccessBlackboardMode() const;

	/**
	 * Get the localized text to display for the specified mode
	 * @param	InMode	The mode to display
	 * @return the localized text representation of the mode
	 */
	static FText GetLocalizedMode(FName InMode);

	/** Access the toolbar builder for this editor */
	TSharedPtr<class FDialoguesEditorToolbar> GetToolbarBuilder() { return ToolbarBuilder; }

	/** Get the dialogue tree we are editing (if any) */
	UDialogues* GetDialogues() const;

	/** Spawns the tab with the update graph inside */
	TSharedRef<SWidget> SpawnProperties();

	/** Spawns the search tab */
	TSharedRef<SWidget> SpawnSearch();

	// @todo This is a hack for now until we reconcile the default toolbar with application modes [duplicated from counterpart in Blueprint Editor]
	void RegisterToolbarTab(const TSharedRef<class FTabManager>& TabManager);

	void RestoreDialogues();

	/** Save the graph state for later editing */
	void SaveEditedObjectState();

	///** Delegate handler for displaying debugger values */
	//FText HandleGetDebugKeyValue(const FName& InKeyName, bool bUseCurrentState) const;

	///** Delegate handler for retrieving timestamp to display */
	//float HandleGetDebugTimeStamp(bool bUseCurrentState) const;

	/** Delegate handler for determining whether to display the current state */
	bool HandleGetDisplayCurrentState() const;

	/** Check whether blackboard mode is current */
	bool HandleIsBlackboardModeActive() const;

	/** Get the currently selected blackboard entry */
	void GetBlackboardSelectionInfo(int32& OutSelectionIndex, bool& bOutIsInherited) const;

	bool CanCreateNewClasses() const;
	TSharedRef<SWidget> HandleCreateNewClassMenu(UClass* BaseClass) const;

	/** Handler for when a node class is picked */
	void HandleNewNodeClassPicked(UClass* InClass) const;

protected:
	/** Called when "Save" is clicked for this asset */
	virtual void SaveAsset_Execute() override;

private:
	/** Create widget for graph editing */
	TSharedRef<class SGraphEditor> CreateGraphEditorWidget(UEdGraph* InGraph);

	/** Creates all internal widgets for the tabs to point at */
	void CreateInternalWidgets();

	/** Add custom menu options */
	void ExtendMenu();

	/** Setup common commands */
	void BindCommonCommands();

	/** Setup commands */
	void BindDebuggerToolbarCommands();

	/** Called when the selection changes in the GraphEditor */
	virtual void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection) override;

	///** Refresh the debugger's display */
	//void RefreshDebugger();

	TSharedPtr<FDocumentTracker> DocumentManager;
	TWeakPtr<FDocumentTabFactory> GraphEditorTabFactoryPtr;

	/* The Dialogues being edited */
	UDialogues* Dialogues;

	/** Property View */
	TSharedPtr<class IDetailsView> DetailsView;

	TSharedPtr<class FDialoguesDebugger> Debugger;

	/** Find results log as well as the search filter */
	TSharedPtr<class SFindInDialogues> FindResults;

	uint32 bForceDisablePropertyEdit : 1;

	TSharedPtr<class FDialoguesEditorToolbar> ToolbarBuilder;

	/** Handle to the registered OnPackageSave delegate */
	FDelegateHandle OnPackageSavedDelegateHandle;

public:
	/** Modes in mode switcher */
	static const FName DialoguesMode;
};