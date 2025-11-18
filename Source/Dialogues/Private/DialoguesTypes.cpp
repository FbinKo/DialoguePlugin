#include "DialoguesTypes.h"
#include "DialoguesComponent.h"

FString FDialogueMessage::ToString() const
{
	return Data.Get()->ToString();
}

float FDialogueMessage::GetDuration() const
{
	return Data.Get()->GetDuration();
}

FDialoguesContext FDialoguesContext::CreateContext(UDialoguesComponent* InOwner, const UDialogueNode_Step* InConsideredStep)
{
	FDialoguesContext Context;
	Context.DialogueComponent = InOwner;
	Context.ConsideredStep = InConsideredStep;

	return Context;
}

FDialoguesContext FDialoguesContext::CreateChildContext(const UDialogueNode_Step* NewConsideredStep) const
{
	FDialoguesContext Context;
	Context.ConsideredStep = NewConsideredStep;

	return Context;
}

UWorld* FDialoguesContext::GetWorld() const
{
	if(DialogueComponent)
		return DialogueComponent->GetWorld();

	return nullptr;
}