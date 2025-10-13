#include "DialoguesGraphModule.h"
#include "DialoguesStyle.h"
#include "UObject/ObjectSaveContext.h"

DEFINE_LOG_CATEGORY(LogDialoguesGraph);

#define LOCTEXT_NAMESPACE "DialoguesEditor"

void FDialoguesGraphModule::StartupModule()
{
	FDialoguesStyle::Initialize();
	FDialoguesStyle::ReloadTextures();

	UPackage::PreSavePackageWithContextEvent.AddRaw(this, &FDialoguesGraphModule::HandlePreSavePackage);

#if WITH_EDITOR
	FEditorDelegates::BeginPIE.AddRaw(this, &FDialoguesGraphModule::HandleBeginPIE);
#endif
}

void FDialoguesGraphModule::ShutdownModule()
{
	UPackage::PreSavePackageWithContextEvent.RemoveAll(this);

#if WITH_EDITOR
	FEditorDelegates::BeginPIE.RemoveAll(this);
#endif

	FDialoguesStyle::Shutdown();
}

void FDialoguesGraphModule::HandlePreSavePackage(UPackage* Package, FObjectPreSaveContext ObjectSaveContext)
{
	TArray<UObject*> Objects;
	const bool bIncludeNestedObjects = false;
	GetObjectsWithPackage(Package, Objects, bIncludeNestedObjects);
	for (UObject* RootPackageObject : Objects)
	{
		/*if (UConversationDatabase* Database = Cast<UConversationDatabase>(RootPackageObject))
		{
			FConversationCompiler::RebuildBank(Database);
		}*/
	}
}

void FDialoguesGraphModule::HandleBeginPIE(bool bIsSimulating)
{
	//FConversationCompiler::ScanAndRecompileOutOfDateCompiledConversations();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDialoguesGraphModule, DialoguesGraph)