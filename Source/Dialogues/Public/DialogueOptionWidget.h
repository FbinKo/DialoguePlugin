// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialoguesTypes.h"
#include "DialogueOptionWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptionSelected, FAdvanceDialogueRequest, SelectedOption);

UCLASS()
class DIALOGUES_API UDialogueOptionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FDialogueOption option;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Option;

	UPROPERTY(BlueprintAssignable)
	FOnOptionSelected OnOptionSelected;
	
public:
	UDialogueOptionWidget(const FObjectInitializer& ObjectInitializer);
	void SetOption(FDialogueOption NewOption);

protected:
	UFUNCTION()
	void OnOptionClicked();
};
