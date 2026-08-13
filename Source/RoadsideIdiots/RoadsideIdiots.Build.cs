using UnrealBuildTool;

public class RoadsideIdiots : ModuleRules
{
    public RoadsideIdiots(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "AIModule",
            "AssetRegistry"
        });
    }
}
