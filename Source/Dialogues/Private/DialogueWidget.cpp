// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueWidget.h"
#include "DialoguesComponent.h"
#include "DialogueOptionWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/Button.h"

UDialogueWidget::UDialogueWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer), StartAnimation(nullptr), AdvanceAnimation(nullptr), EndAnimation(nullptr)
{
	bIsOnlyVisibleWhenSpeaker = false;
}

void UDialogueWidget::NativeConstruct()
{
	if (!DialogueComponent)
	{
		APawn* Pawn = GetOwningPlayerPawn();
		if (OwningPawn)
			Pawn = OwningPawn;

		if (Pawn)
		{
			DialogueComponent = Cast<UDialoguesComponent>(Pawn->FindComponentByClass(UDialoguesComponent::StaticClass()));
			if (DialogueComponent)
			{
				DialogueComponent->OnDialogueStatusChange.AddDynamic(this, &ThisClass::OnDialogueStatusChange);
			}
			else {
				if (Text_Transcript)
					Text_Transcript->SetText(FText::FromString(TEXT("Player doesnt have a DialogueComponent")));
			}

		}
	}
}

void UDialogueWidget::SetOwningPawn(APawn* NewOwningPawn)
{
	if (DialogueComponent)
	{
		DialogueComponent->OnDialogueStatusChange.Broadcast(nullptr);
		DialogueComponent->OnDialogueStatusChange.RemoveDynamic(this, &ThisClass::OnDialogueStatusChange);
		DialogueComponent = nullptr;
	}

	OwningPawn = NewOwningPawn;

	APawn* Pawn = GetOwningPlayerPawn();
	if (OwningPawn)
		Pawn = OwningPawn;

	if (Pawn)
	{
		DialogueComponent = Cast<UDialoguesComponent>(Pawn->FindComponentByClass(UDialoguesComponent::StaticClass()));
		if (DialogueComponent)
		{
			DialogueComponent->OnDialogueStatusChange.AddDynamic(this, &ThisClass::OnDialogueStatusChange);
		}
		else {
			if (Text_Transcript)
				Text_Transcript->SetText(FText::FromString(TEXT("Player doesnt have a DialogueComponent")));
		}

	}
}

void UDialogueWidget::OnOptionSelected(FAdvanceDialogueRequest SelectedOption)
{
	if (ActiveComponent)
		ActiveComponent->AdvanceDialogue(SelectedOption);
}

void UDialogueWidget::OnDialogueUpdate_Implementation(const FDialogueUpdatePayload& Payload)
{
	if (bIsOnlyVisibleWhenSpeaker)
		if (DialogueComponent->GetInfo().ParticipantID == Payload.Message.SpeakerID)
			SetVisibility(ESlateVisibility::Visible);
		else
			SetVisibility(ESlateVisibility::Hidden);

	if (Text_Speaker)
		Text_Speaker->SetText(Payload.SpeakerInfo.Name);

	if (Image_Speaker)
		Image_Speaker->SetBrushFromMaterial(Payload.SpeakerInfo.image);

	UDialogueData_AudioAndText* data = Payload.Message.GetData<UDialogueData_AudioAndText>();
	if (Text_Transcript)
		Text_Transcript->SetText(data->Text);

	OptionsContainer->ClearChildren();
	if (Payload.Options.IsEmpty())
	{
		UDialogueOptionWidget* NewOption = Cast<UDialogueOptionWidget>(CreateWidget(GetOwningPlayer(), OptionsClass));
		NewOption->SetOption(FDialogueOption());
		NewOption->OnOptionSelected.AddDynamic(this, &ThisClass::OnOptionSelected);
		OptionsContainer->AddChild(NewOption);
	}
	else {
		for (FDialogueOption option : Payload.Options)
		{
			UDialogueOptionWidget* NewOption = Cast<UDialogueOptionWidget>(CreateWidget(GetOwningPlayer(), OptionsClass));
			NewOption->SetOption(option);
			NewOption->OnOptionSelected.AddDynamic(this, &ThisClass::OnOptionSelected);
			OptionsContainer->AddChild(NewOption);
		}
	}
}

void UDialogueWidget::OnDialogueStatusChange_Implementation(UDialoguesComponent* ControllingComponent)
{
	if (ActiveComponent != ControllingComponent)
	{
		if (!ActiveComponent)
		{
			ControllingComponent->OnUpdateDialogue.AddDynamic(this, &ThisClass::OnDialogueUpdate);
			PlayAnimation(StartAnimation);
		}
		else {
			ActiveComponent->OnUpdateDialogue.RemoveDynamic(this, &ThisClass::OnDialogueUpdate);
			PlayAnimation(EndAnimation);
		}
		ActiveComponent = ControllingComponent;
	}
}
