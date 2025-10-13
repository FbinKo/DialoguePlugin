#pragma once

#include "AIGraph.h"
#include "Containers/Set.h"
#include "CoreMinimal.h"
#include "HAL/Platform.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"

#include "DialoguesGraph.generated.h"

class UEdGraphNode;
class UObject;

UCLASS()
class DIALOGUESGRAPH_API UDialoguesGraph : public UAIGraph
{
	GENERATED_UCLASS_BODY()

//	enum EUpdateFlags
//	{
//		RebuildGraph = 0,
//		ClearDebuggerFlags = 1,
//		KeepRebuildCounter = 2,
//	};

	void OnSave();

	virtual void UpdateAsset(int32 UpdateFlags = 0);

	void CreateDialoguesFromGraph();
protected:
#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif
};