#pragma once

#include "AIGraphNode.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "HAL/Platform.h"
#include "Internationalization/Text.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"

#include "DialoguesGraphNode.generated.h"

class UEdGraph;
class UEdGraphSchema;
class UObject;
template <typename T> struct TObjectPtr;

UCLASS()
class DIALOGUESGRAPH_API UDialoguesGraphNode : public UAIGraphNode
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UEdGraphNode Interface
	virtual class UDialoguesGraph* GetDialoguesGraph();
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//virtual FText GetTooltipText() const override;
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const override;
	virtual FLinearColor GetNodeBodyTintColor() const override;
	virtual UObject* GetJumpTargetForDoubleClick() const override;
	virtual bool CanJumpToDefinition() const override;
	virtual void JumpToDefinition() const override;
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	//~ End UEdGraphNode Interface

#if WITH_EDITOR
	//virtual void PostEditUndo() override;
#endif
	virtual FName GetNameIcon() const;
	//~ Begin UAIGraphNode Interface
	virtual FText GetDescription() const override;
	//virtual bool HasErrors() const override;
	virtual void InitializeInstance() override;
	//~ End UAIGraphNode Interface
	template<class T>
	T* GetRuntimeNode() const
	{
		return Cast<T>(NodeInstance);
	}

	virtual bool CanPlaceBreakpoints() const { return false; }

	//	void ClearDebuggerState();

	//	/** highlighting nodes in abort range for more clarity when setting up decorators */
	//	uint32 bHighlightInAbortRange0 : 1;
	//
	//	/** highlighting nodes in abort range for more clarity when setting up decorators */
	//	uint32 bHighlightInAbortRange1 : 1;
	//
	//	/** highlighting connections in search range for more clarity when setting up decorators */
	//	uint32 bHighlightInSearchRange0 : 1;
	//
	//	/** highlighting connections in search range for more clarity when setting up decorators */
	//	uint32 bHighlightInSearchRange1 : 1;
	//
	//	/** highlighting nodes during quick find */
	//	uint32 bHighlightInSearchTree : 1;
	//
	//	/** debugger flag: breakpoint exists */
	//	uint32 bHasBreakpoint : 1;
	//
	//	/** debugger flag: breakpoint is enabled */
	//	uint32 bIsBreakpointEnabled : 1;
	//
	//	/** debugger flag: mark node as active (current state) */
	//	uint32 bDebuggerMarkCurrentlyActive : 1;
	//
	//	/** debugger flag: mark node as active (browsing previous states) */
	//	uint32 bDebuggerMarkPreviouslyActive : 1;
	//
	//	/** debugger flag: briefly flash active node */
	//	uint32 bDebuggerMarkFlashActive : 1;
	//
	//	/** debugger flag: mark as succeeded search path */
	//	uint32 bDebuggerMarkSearchSucceeded : 1;
	//
	//	/** debugger flag: mark as failed on search path */
	//	uint32 bDebuggerMarkSearchFailed : 1;
	//
	//	/** debugger flag: mark as trigger of search path */
	//	uint32 bDebuggerMarkSearchTrigger : 1;
	//
	//	/** debugger flag: mark as trigger of discarded search path */
	//	uint32 bDebuggerMarkSearchFailedTrigger : 1;
	//
	//	/** debugger flag: mark as going to parent */
	//	uint32 bDebuggerMarkSearchReverseConnection : 1;
	//
	//	/** debugger flag: mark stopped on this breakpoint */
	//	uint32 bDebuggerMarkBreakpointTrigger : 1;
	//
	//	/** debugger variable: index on search path */
	//	int32 DebuggerSearchPathIndex;
	//
	//	/** debugger variable: number of nodes on search path */
	//	int32 DebuggerSearchPathSize;
	//
	//	/** debugger variable: incremented on change of debugger flags for render updates */
	//	int32 DebuggerUpdateCounter;
	//
	//	/** used to show node's runtime description rather than static one */
	//	FString DebuggerRuntimeDescription;
};
