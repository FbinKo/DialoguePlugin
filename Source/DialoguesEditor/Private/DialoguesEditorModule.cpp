#include "DialoguesEditorModule.h"
#include "AssetTypeActions_Dialogues.h"
#include "DialoguesEditor.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "EdGraphUtilities.h"
#include "DialoguesGraphNode.h"
#include "SGraphNode_DialoguesGraph.h"
#include "DialoguesGraphTypes.h"
#include "DialogueNode.h"
#include "DialogueNode_Step.h"

#define LOCTEXT_NAMESPACE "DialoguesEditorModule"

const FName FDialoguesEditorModule::DialoguesEditorAppIdentifier(TEXT("DialoguesEditorApp"));

class FGraphPanelNodeFactory_Dialogues : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override
	{
		if (UDialoguesGraphNode* GraphNode = Cast<UDialoguesGraphNode>(Node))
		{
			return SNew(SGraphNode_Dialogues, GraphNode);
		}

		return NULL;
	}
};

TSharedPtr<FGraphPanelNodeFactory> GraphPanelNodeFactory_Dialogues;

void FDialoguesEditorModule::StartupModule()
{
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);

	GraphPanelNodeFactory_Dialogues = MakeShareable(new FGraphPanelNodeFactory_Dialogues());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_Dialogues);

	// register dialogues category
	IAssetTools& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	DialoguesAssetCategoryBit = AssetToolsModule.RegisterAdvancedAssetCategory(FName(TEXT("Dialogues")), LOCTEXT("DialoguesAssetCategory", "Dialogues"));
	TSharedPtr<FAssetTypeActions_Dialogues> DialoguesAssetTypeAction = MakeShareable(new FAssetTypeActions_Dialogues(DialoguesAssetCategoryBit));
	ItemDataAssetTypeActions.Add(DialoguesAssetTypeAction);
	AssetToolsModule.RegisterAssetTypeActions(DialoguesAssetTypeAction.ToSharedRef());
}


void FDialoguesEditorModule::ShutdownModule()
{
	if (!UObjectInitialized())
	{
		return;
	}

	MenuExtensibilityManager.Reset();
	ToolBarExtensibilityManager.Reset();

	if (GraphPanelNodeFactory_Dialogues.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_Dialogues);
		GraphPanelNodeFactory_Dialogues.Reset();
	}

	// Unregister the Dialogues item data asset type actions
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& AssetTypeAction : ItemDataAssetTypeActions)
		{
			if (AssetTypeAction.IsValid())
			{
				AssetToolsModule.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
			}
		}
	}
	ItemDataAssetTypeActions.Empty();

	// Unregister the details customization
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		/*PropertyModule.UnregisterCustomPropertyTypeLayout("BlackboardKeySelector");
		PropertyModule.UnregisterCustomClassLayout("BTDecorator_Blackboard");
		PropertyModule.UnregisterCustomClassLayout("BTDecorator");
		PropertyModule.NotifyCustomizationModuleChanged();*/
	}
}

TSharedRef<IDialoguesEditor> FDialoguesEditorModule::CreateDialoguesEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UObject* Object)
{
	TSharedRef< FDialoguesEditor > NewDialoguesEditor(new FDialoguesEditor());
	NewDialoguesEditor->InitDialoguesEditor(Mode, InitToolkitHost, Object);
	return NewDialoguesEditor;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDialoguesEditorModule, DialoguesEditor)