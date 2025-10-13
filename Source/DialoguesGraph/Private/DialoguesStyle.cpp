// Copyright Epic Games, Inc. All Rights Reserved.

#include "DialoguesStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FDialoguesStyle::StyleInstance = nullptr;

void FDialoguesStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FDialoguesStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FDialoguesStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("DialoguesStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon24x24(24.0f, 24.0f);

TSharedRef< FSlateStyleSet > FDialoguesStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("DialoguesStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("Dialogues")->GetBaseDir() / TEXT("Resources"));

	Style->Set("Dialogues.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	Style->Set("Dialogues.Graph.Icon", new IMAGE_BRUSH(TEXT("Graph/Icon"), Icon24x24));
	Style->Set("Dialogues.Graph.Node.Requirement.Icon", new IMAGE_BRUSH(TEXT("Graph/RequirementIcon"), Icon24x24));
	Style->Set("Dialogues.Graph.Node.Step.Icon", new IMAGE_BRUSH(TEXT("Graph/StepIcon"), Icon24x24));

	return Style;
}

void FDialoguesStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FDialoguesStyle::Get()
{
	return *StyleInstance;
}
