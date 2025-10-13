#pragma once

#include "DialoguesTypes.generated.h"

class UDialogueNode_Step;
class UDialogueComponent;

UENUM(BlueprintType)
enum class ERequirementResult : uint8
{
	Passed,
	FailedButVisible,
	FailedAndHidden,
};

UENUM(BlueprintType)
enum class EDialogueStepResult : uint8
{
	Invalid,
	Abort,
	/** Advances the dialogue to the next step, or a random one if there are multiple. */
	Advance,
	Pause,
	ReturnToLastDecision,
	/**
	 * Does not advance the dialogue, just refreshes the current decisions again.
	 * This option is really useful if you need to have the user make a choice and then
	 * make the same choice again, ex. User clicks an option to buy an item, and you want
	 * them to be able to repeat that action.
	 */
	ReturnToCurrentDecision,
	ReturnToStart
};

USTRUCT(BlueprintType)
struct DIALOGUES_API FDialogueParticipant
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = Dialogue)
	TObjectPtr<AActor> Actor = nullptr;

	FDialogueParticipant() {}
	FDialogueParticipant(AActor* Participant) : Actor(Participant) {}

	template<class TDialogueComponentClass = UDialoguesComponent>
	TDialogueComponentClass* GetDialogueComponent() const
	{
		return Actor ? Actor->FindComponentByClass<TDialogueComponentClass>() : nullptr;
	}

	bool operator ==(const FDialogueParticipant& Other) const
	{
		return Actor == Other.Actor;
	}
};

USTRUCT(BlueprintType)
struct DIALOGUES_API FAdvanceDialogueRequest
{
	GENERATED_BODY()

public:
	FAdvanceDialogueRequest() {}
	FAdvanceDialogueRequest(UDialogueNode_Step* InNextStep) : NextStep(InNextStep) {}

public:
	FString ToString() const;

public:
	UPROPERTY(BlueprintReadWrite, Category = Dialogue)
	UDialogueNode_Step* NextStep = nullptr;
};

USTRUCT(BlueprintType)
struct DIALOGUES_API FDialogueMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = Dialogue)
	FGameplayTag SpeakerID;

	UPROPERTY(BlueprintReadWrite, Category = Dialogue)
	bool bContinuesAutomatically = false;

	UPROPERTY(BlueprintReadWrite, Category = Dialogue)
	TObjectPtr<UDialogueData> Data;

	FString ToString() const;
	float GetDuration() const;

	template<class TDataClass = UDialogueData>
	TDataClass* GetData() const
	{
		return Cast<TDataClass>(Data);
	}
};

USTRUCT(BlueprintType)
struct DIALOGUES_API FDialogueStepResult
{
	GENERATED_BODY()

public:
	static FDialogueStepResult Abort()
	{
		return FDialogueStepResult(EDialogueStepResult::Abort, FAdvanceDialogueRequest(), FDialogueMessage());
	}

	static FDialogueStepResult Advance(const FAdvanceDialogueRequest& InAdvanceToChoice)
	{
		return FDialogueStepResult(EDialogueStepResult::Advance, InAdvanceToChoice, FDialogueMessage());
	}

	static FDialogueStepResult Pause(const FDialogueMessage& InMessage)
	{
		return FDialogueStepResult(EDialogueStepResult::Pause, FAdvanceDialogueRequest(), InMessage);
	}

	static FDialogueStepResult ReturnToLastDecision()
	{
		return FDialogueStepResult(EDialogueStepResult::ReturnToLastDecision, FAdvanceDialogueRequest(), FDialogueMessage());
	}

	static FDialogueStepResult ReturnToCurrentStep()
	{
		return FDialogueStepResult(EDialogueStepResult::ReturnToCurrentDecision, FAdvanceDialogueRequest(), FDialogueMessage());
	}

	static FDialogueStepResult ReturnToStart()
	{
		return FDialogueStepResult(EDialogueStepResult::ReturnToStart, FAdvanceDialogueRequest(), FDialogueMessage());
	}

	EDialogueStepResult GetType() const { return Type; }
	const FAdvanceDialogueRequest& GetNextStep() const { ensure(Type == EDialogueStepResult::Advance); return NextStep; }
	const FDialogueMessage& GetMessage() const { ensure(Type == EDialogueStepResult::Pause); return Message; }

	bool CanConversationContinue() const
	{
		return !(Type == EDialogueStepResult::Invalid || Type == EDialogueStepResult::Abort);
	}

public:
	FDialogueStepResult() : Type(EDialogueStepResult::Invalid) {}

	/**
	 * Constructor
	 *
	 * @param EForceInit Force init enum
	 */
	explicit FORCEINLINE FDialogueStepResult(EForceInit)
		: Type(EDialogueStepResult::Invalid)
	{
	}

private:
	FDialogueStepResult(EDialogueStepResult InType, const FAdvanceDialogueRequest& InNextStep, const FDialogueMessage& InMessage)
		: Type(InType)
		, NextStep(InNextStep)
		, Message(InMessage)
	{
	}

	UPROPERTY()
	EDialogueStepResult Type;

	UPROPERTY()
	FAdvanceDialogueRequest NextStep;

	UPROPERTY()
	FDialogueMessage Message;
};

USTRUCT(BlueprintType)
struct DIALOGUES_API FDialoguesContext
{
	GENERATED_BODY()

public:

	static FDialoguesContext CreateContext(UDialoguesComponent* InOwner, const UDialogueNode_Step* InConsideredStep);

	FDialoguesContext CreateChildContext(const UDialogueNode_Step* NewConsideredStep) const;

	UWorld* GetWorld() const;

	const UDialogueNode_Step* GetTaskBeingConsidered() const { return ConsideredStep; }

	UDialoguesComponent* GetDialogueComponent() const { return DialogueComponent; }

private:
	UPROPERTY()
	TObjectPtr<UDialoguesComponent> DialogueComponent = nullptr;

	UPROPERTY()
	TObjectPtr<const UDialogueNode_Step> ConsideredStep = nullptr;
};

USTRUCT(BlueprintType)
struct DIALOGUES_API FDialogueParticipantInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	FGameplayTag ParticipantID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FText Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	TObjectPtr<UMaterial> image;
};

USTRUCT(BlueprintType)
struct DIALOGUES_API FDialogueOption
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ERequirementResult Access;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDialogueNode_Step> NextStep;
};

USTRUCT(BlueprintType)
struct DIALOGUES_API FDialogueUpdatePayload
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FDialogueMessage Message;

	UPROPERTY(BlueprintReadWrite)
	FDialogueParticipantInfo SpeakerInfo;

	UPROPERTY(BlueprintReadWrite)
	TArray<FDialogueParticipant> Participants;

	UPROPERTY(BlueprintReadWrite)
	UDialogueNode_Step* CurrentNode;

	UPROPERTY(BlueprintReadWrite)
	TArray<FDialogueOption> Options;

	FString ToString() const { return Message.ToString(); }
};

UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class DIALOGUES_API UDialogueData : public UObject
{
	GENERATED_BODY()

public:
	// only relevant if no asset with time is referenced
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Dialogue)
	float Duration = 3.f;

public:
	virtual void OnInstanceCreated(UDialogueComponent* Component) const {}
	virtual FString ToString() const { return ""; }
	virtual float GetDuration() const { return Duration; }
};

UCLASS(BlueprintType)
class UDialogueData_AudioAndText : public UDialogueData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Dialogue)
	FText Text;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Dialogue)
	TObjectPtr<class USoundBase> Audio;

public:
	virtual FString ToString() const override { return Text.ToString(); }
	virtual float GetDuration() const override { return Audio ? Audio.Get()->GetDuration() : Duration; }
};

UCLASS(BlueprintType)
class UDialogueData_Animation : public UDialogueData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Dialogue)
	TObjectPtr<class UAnimMontage> Animation;

public:
	virtual float GetDuration() const override { return Animation ? Animation.Get()->CalculateSequenceLength() : Duration; }
};