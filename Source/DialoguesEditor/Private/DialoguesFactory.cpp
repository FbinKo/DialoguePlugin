#include "DialoguesFactory.h"

#include "Dialogues.h"
#include "Misc/AssertionMacros.h"
#include "Templates/SubclassOf.h"
#include "UObject/Class.h"

class FFeedbackContext;
class UObject;

#define LOCTEXT_NAMESPACE "DialoguesFactory"

UDialoguesFactory::UDialoguesFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UDialogues::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

bool UDialoguesFactory::CanCreateNew() const
{
	return true;
}

UObject* UDialoguesFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UDialogues::StaticClass()));
	return NewObject<UDialogues>(InParent, Class, Name, Flags);;
}

#undef LOCTEXT_NAMESPACE
