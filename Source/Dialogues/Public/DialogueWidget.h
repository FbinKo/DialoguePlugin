// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidget.generated.h"

struct FDialogueUpdatePayload;
class UDialoguesComponent;
class UTextBlock;
class UImage;
class UPanelWidget;

UCLASS(Abstract, Blueprintable)
class DIALOGUES_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnim))
	UWidgetAnimation* StartAnimation;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnim))
	UWidgetAnimation* AdvanceAnimation;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnim))
	UWidgetAnimation* EndAnimation;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Speaker;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Image_Speaker;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Transcript;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> OptionsContainer;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> OptionsClass;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APawn> OwningPawn;

protected:
	UPROPERTY(BlueprintReadOnly)
	UDialoguesComponent* DialogueComponent;

	UPROPERTY(BlueprintReadOnly)
	UDialoguesComponent* ActiveComponent;

	UPROPERTY(EditDefaultsOnly)
	bool bIsOnlyVisibleWhenSpeaker;

public:
	UDialogueWidget(const FObjectInitializer& ObjectInitializer);
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintNativeEvent)
	void OnDialogueUpdate(const FDialogueUpdatePayload& Payload);

	UFUNCTION(BlueprintNativeEvent)
	void OnDialogueStatusChange(UDialoguesComponent* ControllingComponent);

	UFUNCTION(BlueprintCallable)
	void SetOwningPawn(APawn* NewOwningPawn);

protected:
	UFUNCTION()
	void OnOptionSelected(FAdvanceDialogueRequest SelectedOption);
};
