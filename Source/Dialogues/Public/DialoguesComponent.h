#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DialoguesTypes.h"
#include "DialoguesComponent.generated.h"

class UDialogues;
class UDialogueNode_Step;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDialogueStatusChange, UDialoguesComponent*, activeComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDialogueUpdate, const FDialogueUpdatePayload&, payload);

UCLASS(BlueprintType, hidecategories = (Sockets, Collision), meta = (BlueprintSpawnableComponent))
class DIALOGUES_API UDialoguesComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FDialogueStatusChange OnDialogueStatusChange;
	UPROPERTY(BlueprintAssignable)
	FDialogueUpdate OnUpdateDialogue;

protected:
	UPROPERTY(EditDefaultsOnly)
	FDialogueParticipantInfo DialogueInfo;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UDialogues> Dialogue;

	UPROPERTY()
	UDialogueNode_Step* StartStep;
	UPROPERTY()
	UDialogueNode_Step* CurrentStep;
	UPROPERTY()
	TArray<UDialogueNode_Step*> DecisionsTree;

	FRandomStream DialogueStream;

	TArray<FDialogueParticipant> Participants;

	bool bIsInDialogue;
	FTimerHandle AutoAdvanceHandle;

public:
	virtual FString GetDebugInfoString() const { return TEXT(""); }

	FDialogueParticipantInfo GetInfo() { return DialogueInfo; }

#if ENABLE_VISUAL_LOG
	virtual void DescribeSelfToVisLog(struct FVisualLogEntry* Snapshot) const;
#endif // ENABLE_VISUAL_LOG

	/** BEGIN UActorComponent overrides */
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** END UActorComponent overrides */

	void ResetDialogue();

	UFUNCTION(BlueprintCallable)
	virtual void StartDialogue(FGameplayTag Start, TArray<FDialogueParticipant> OtherParticipants);

	UFUNCTION(BlueprintCallable)
	virtual void AdvanceDialogue(const FAdvanceDialogueRequest& NextStep);
	virtual void PauseDialogue(const FDialogueMessage& Message);

	void AutoAdvance();

	virtual void ReturnToLastDecision();
	virtual void ReturnToCurrentDecision();
	virtual void ReturnToStart();

	UFUNCTION(BlueprintCallable)
	virtual void StopDialogue(const FString& Reason);

	/** dialogue won't be needed anymore, stop all activity and run cleanup */
	virtual void Cleanup() {}

	UFUNCTION(BlueprintPure)
	virtual bool IsInDialogue() const;

protected:
	void UpdateCurrentStep(UDialogueNode_Step* NewStep);
	FDialogueParticipantInfo GetParticipantInfo(FGameplayTag ID);

};