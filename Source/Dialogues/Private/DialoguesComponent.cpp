#include "DialoguesComponent.h"
#include "Dialogues.h"
#include "DialogueNode_Step.h"

UDialoguesComponent::UDialoguesComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsInDialogue = false;
}

void UDialoguesComponent::DescribeSelfToVisLog(FVisualLogEntry* Snapshot) const
{
}

void UDialoguesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDialoguesComponent::ResetDialogue()
{
	StartStep = nullptr;
	CurrentStep = nullptr;
	DecisionsTree.Reset();
}

void UDialoguesComponent::StartDialogue(FGameplayTag Start, TArray<FDialogueParticipant> InParticipants)
{
	if (!Dialogue || bIsInDialogue || InParticipants.IsEmpty())
		return;

	Participants = InParticipants;
	Participants.AddUnique(FDialogueParticipant(GetOwner()));

	FDialoguesContext Context = FDialoguesContext::CreateContext(this, nullptr);
	if (FDialogueEntryList* potentialEntryPoints = Dialogue->EntryNodeMap.Find(Start))
	{
		TArray<UDialogueNode_Step*> potentialStartNodes;
		for (UDialogueNode_Step* node : potentialEntryPoints->DestinationList)
		{
			if (node->CheckRequirements(Context) == ERequirementResult::Passed)
				potentialStartNodes.Add(node);
		}

		if (potentialStartNodes.IsEmpty())
		{
			StopDialogue(TEXT("Abort because no start found for tag: %s"));
			return;
		}
		else
		{
			const int32 id = DialogueStream.RandRange(0, potentialStartNodes.Num() - 1);
			StartStep = potentialStartNodes[id];
		}

		// give feedback to other party that dialogue started so it can visually update -> eg. show UI, but its not necessary because it can all be controlled from component holder as well (just simpler when adding ui when target is informed)
		//		but if 3d ui which is locked to component holder not required or if only audio or animation
		for (FDialogueParticipant participant : Participants)
		{
			if (UDialoguesComponent* participantComponent = participant.GetDialogueComponent())
				participantComponent->OnDialogueStatusChange.Broadcast(this);
		}

		bIsInDialogue = true;

		DialogueStream.Initialize(NAME_None);
		UpdateCurrentStep(StartStep);
	}
}

void UDialoguesComponent::AdvanceDialogue(const FAdvanceDialogueRequest& NextStep)
{
	if (CurrentStep)
	{
		TArray<UDialogueNode_Step*> potentialStartNodes;
		if (NextStep.NextStep)
		{
			if (CurrentStep->NextSteps.Contains(NextStep.NextStep))
			{
				potentialStartNodes.Add(NextStep.NextStep);
			}
			else {
				StopDialogue(TEXT("Abort because selected option is not part of currentStep"));
				return;
			}
		}
		else
		{
			// leaf nodes can either end dialogue or go back to last decision, non leaf nodes result in random selection
			if (CurrentStep->IsDialogueExit()) {
				StopDialogue(TEXT("Reached Exit"));
				return;
			}
			else {
				if (CurrentStep->NextSteps.IsEmpty())
				{
					ReturnToLastDecision();
					return;
				}
				else {
					potentialStartNodes.Append(CurrentStep->NextSteps);
				}
			}
		}

		TArray<UDialogueNode_Step*> validStartNodes;
		FDialoguesContext Context = FDialoguesContext::CreateContext(this, nullptr);
		for (UDialogueNode_Step* node : potentialStartNodes)
		{
			if (node->CheckRequirements(Context) == ERequirementResult::Passed)
				validStartNodes.Add(node);
		}

		if (validStartNodes.IsEmpty())
			StopDialogue(TEXT("Abort because somehow selected an invalid option to advance - need to figure out how that was possible, every case should be covered"));
		else {
			const int32 id = DialogueStream.RandRange(0, validStartNodes.Num() - 1);
			UpdateCurrentStep(validStartNodes[id]);
		}
	}
}

void UDialoguesComponent::PauseDialogue(const FDialogueMessage& Message)
{
	FDialogueUpdatePayload message = FDialogueUpdatePayload();
	message.Message = Message;
	message.SpeakerInfo = GetParticipantInfo(Message.SpeakerID);
	message.Participants = Participants;
	message.CurrentNode = CurrentStep;

	TArray<FDialogueOption> options;
	FDialoguesContext Context = FDialoguesContext::CreateContext(this, nullptr);
	for (UDialogueNode_Step* step : CurrentStep->GetOptions())
	{
		FDialogueOption option;
		option.NextStep = step;
		option.Access = step->CheckRequirements(Context);
		if (option.Access <= ERequirementResult::FailedButVisible)
			options.Add(option);
	}
	message.Options = options;

	DecisionsTree.AddUnique(CurrentStep);

	for (const FDialogueParticipant& participant : Participants)
	{
		if (UDialoguesComponent* participantComponent = participant.GetDialogueComponent())
			participantComponent->OnUpdateDialogue.Broadcast(message);
	}

	if (Message.bContinuesAutomatically)
	{
		GetWorld()->GetTimerManager().SetTimer(AutoAdvanceHandle, this, &ThisClass::AutoAdvance, Message.GetDuration(), false);
	}
}

void UDialoguesComponent::AutoAdvance()
{
	AdvanceDialogue(FAdvanceDialogueRequest());
}

void UDialoguesComponent::ReturnToLastDecision()
{
	if (DecisionsTree.Num() > 1)
	{
		DecisionsTree.Pop();
		UpdateCurrentStep(DecisionsTree.Last());
	}
	else
		ReturnToStart();
}

void UDialoguesComponent::ReturnToCurrentDecision()
{
	if (!DecisionsTree.IsEmpty())
	{
		UDialogueNode_Step* target = DecisionsTree.Pop();
		UpdateCurrentStep(target);
	}
}

void UDialoguesComponent::ReturnToStart()
{
	UDialogueNode_Step* start = StartStep;
	ResetDialogue();
	UpdateCurrentStep(start);
}

void UDialoguesComponent::StopDialogue(const FString& Reason)
{
	if (bIsInDialogue)
	{
		//TODO should probably have a differentiation if abort or only stop
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Reason);

		for (FDialogueParticipant participant : Participants)
		{
			if (UDialoguesComponent* participantComponent = participant.GetDialogueComponent())
				participantComponent->OnDialogueStatusChange.Broadcast(nullptr);
		}
		Participants.Reset();
	}

	ResetDialogue();

	bIsInDialogue = false;
}

bool UDialoguesComponent::IsInDialogue() const
{
	return bIsInDialogue;
}

void UDialoguesComponent::UpdateCurrentStep(UDialogueNode_Step* NewStep)
{
	FDialoguesContext Context = FDialoguesContext::CreateContext(this, nullptr);
	CurrentStep = NewStep;

	if (CurrentStep)
	{
		const FDialogueStepResult result = CurrentStep->ExecuteStep(Context);
		switch (result.GetType())
		{
		case EDialogueStepResult::Abort:
			StopDialogue(TEXT("Step returned Abort"));
			break;
		case EDialogueStepResult::Advance:
			AdvanceDialogue(result.GetNextStep());
			break;
		case EDialogueStepResult::Pause:
			PauseDialogue(result.GetMessage());
			break;
		case EDialogueStepResult::ReturnToLastDecision:
			ReturnToLastDecision();
			break;
		case EDialogueStepResult::ReturnToCurrentDecision:
			ReturnToCurrentDecision();
			break;
		case EDialogueStepResult::ReturnToStart:
			ReturnToStart();
			break;
		default:
			break;
		}
	}
	else
		StopDialogue(TEXT("UpdateCurrentStep with nullptr"));
}

FDialogueParticipantInfo UDialoguesComponent::GetParticipantInfo(FGameplayTag ID)
{
	for (FDialogueParticipant participant : Participants)
	{
		if (UDialoguesComponent* participantComponent = participant.GetDialogueComponent())
			if (participantComponent->GetInfo().ParticipantID == ID)
				return participantComponent->GetInfo();
	}
	return FDialogueParticipantInfo();
}
