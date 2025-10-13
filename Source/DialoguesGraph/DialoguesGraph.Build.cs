using UnrealBuildTool;

public class DialoguesGraph : ModuleRules
{
    public DialoguesGraph(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core"
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Projects",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Dialogues",
                "GraphEditor",
                "AIGraph",
                "AIModule",
                "UnrealEd",
                "KismetWidgets",
                "ToolMenus",
                "PropertyEditor",
                "GameplayTags",
				// ... add private dependencies that you statically link with here ...	
			}
            );
    }
}