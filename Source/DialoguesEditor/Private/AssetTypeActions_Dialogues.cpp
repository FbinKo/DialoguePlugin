#include "AssetTypeActions_Dialogues.h"
#include "Dialogues.h"
#include "DialoguesEditor.h"
#include "DialoguesEditorModule.h"

#include "AIModule.h"

FAssetTypeActions_Dialogues::FAssetTypeActions_Dialogues(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

UClass* FAssetTypeActions_Dialogues::GetSupportedClass() const
{
    return UDialogues::StaticClass();
}

void FAssetTypeActions_Dialogues::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto Object : InObjects)
	{
		auto Dialogues = Cast<UDialogues>(Object);
		if (Dialogues != nullptr)
		{
			// check if we have an editor open for this BT's blackboard & use that if we can
			bool bFoundExisting = false;
			/*if (Dialogues->BlackboardAsset != nullptr)
			{
				const bool bFocusIfOpen = false;
				FDialoguesEditor* ExistingInstance = static_cast<FDialoguesEditor*>(GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Dialogues->BlackboardAsset, bFocusIfOpen));
				if (ExistingInstance != nullptr && ExistingInstance->GetDialogues() == nullptr)
				{
					ExistingInstance->InitDialoguesEditor(Mode, EditWithinLevelEditor, Dialogues);
					bFoundExisting = true;
				}
			}*/

			if (!bFoundExisting)
			{
				FDialoguesEditorModule& DialoguesEditorModule = FModuleManager::GetModuleChecked<FDialoguesEditorModule>("DialoguesEditor");
				TSharedRef< IDialoguesEditor > NewEditor = DialoguesEditorModule.CreateDialoguesEditor(Mode, EditWithinLevelEditor, Dialogues);
			}
		}
	}
}

uint32 FAssetTypeActions_Dialogues::GetCategories()
{
    return AssetCategory;
}

void FAssetTypeActions_Dialogues::PerformAssetDiff(UObject* Asset1, UObject* Asset2, const FRevisionInfo& OldRevision, const FRevisionInfo& NewRevision) const
{
}
