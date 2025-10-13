// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogueOptionWidget.h"
#include "DialoguesTypes.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "DialogueNode_Step.h"

UDialogueOptionWidget::UDialogueOptionWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UDialogueOptionWidget::SetOption(FDialogueOption NewOption)
{
	option = NewOption;

	if (Button)
	{
		Button->SetIsEnabled(option.Access == ERequirementResult::Passed);
		Button->OnClicked.AddDynamic(this, &ThisClass::OnOptionClicked);
	}

	if (Text_Option)
	{
		FText newText = NSLOCTEXT("DialogueOption", "DialogueOption_DefaultText", "Continue");
		if (option.NextStep)
		{
			if (option.NextStep->ContinuesAutomatically())
				return;

			newText = option.NextStep->Title;
		}

		Text_Option->SetText(newText);
	}
}

void UDialogueOptionWidget::OnOptionClicked()
{
	OnOptionSelected.Broadcast(FAdvanceDialogueRequest(option.NextStep));
}
