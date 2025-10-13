using UnrealBuildTool;

public class DialoguesEditor : ModuleRules
{
    public DialoguesEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "AIModule",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Dialogues",
                "DialoguesGraph",
                "AIGraph",
                "UnrealEd",
                "GraphEditor",
                "PropertyEditor",
                "Kismet",
                "InputCore",
                "ApplicationCore",
				// ... add private dependencies that you statically link with here ...	
			}
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }
}