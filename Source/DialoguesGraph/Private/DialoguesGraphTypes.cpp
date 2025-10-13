#include "DialoguesGraphTypes.h"

#include "UObject/NameTypes.h"

const FName UDialogueGraphTypes::PinCategory_MultipleNodes("MultipleNodes");
const FName UDialogueGraphTypes::PinCategory_SingleComposite("SingleComposite");
const FName UDialogueGraphTypes::PinCategory_SingleStep("SingleStep");
const FName UDialogueGraphTypes::PinCategory_SingleNode("SingleNode");

UDialogueGraphTypes::UDialogueGraphTypes(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}