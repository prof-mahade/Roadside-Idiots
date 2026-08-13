using UnrealBuildTool;
using System.Collections.Generic;

public class RoadsideIdiotsTarget : TargetRules
{
    public RoadsideIdiotsTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("RoadsideIdiots");
    }
}
