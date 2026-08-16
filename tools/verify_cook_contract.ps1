param(
    [string]$ProjectRoot = "C:\GameDev\Roadside-Idiots"
)

$ErrorActionPreference = "Stop"

$DefaultGameIni = Join-Path $ProjectRoot "Config\DefaultGame.ini"

Write-Host "Roadside Idiots - Cook Contract Verification" -ForegroundColor Cyan

if (-not (Test-Path $DefaultGameIni)) {
    Write-Host "[FAIL] Missing $DefaultGameIni" -ForegroundColor Red
    exit 2
}

$Failures = @()
$Lines = Get-Content $DefaultGameIni

function Get-CookPaths([string]$Directive) {
    $Pattern = '^\+' + [regex]::Escape($Directive) + '=\(Path="([^"]+)"\)'
    $Paths = @()
    foreach ($Line in $Lines) {
        $Match = [regex]::Match($Line.Trim(), $Pattern)
        if ($Match.Success) {
            $Paths += $Match.Groups[1].Value.TrimEnd('/')
        }
    }
    return @($Paths)
}

$AlwaysCook = @(Get-CookPaths "DirectoriesToAlwaysCook")
$NeverCook = @(Get-CookPaths "DirectoriesToNeverCook")

$RequiredAlwaysCook = @(
    "/Game/MotoInteractionAnims/Animations",
    "/Game/MotoInteractionAnims/Demo/Bike/Mesh",
    "/Game/Characters/Mannequins/Meshes",
    "/Game/PN_Banana/Meshes/plants",
    "/Game/PN_tropicalGroundPlants/Meshes"
)

foreach ($Required in $RequiredAlwaysCook) {
    if ($AlwaysCook -contains $Required) {
        Write-Host "[PASS] Required cook root: $Required" -ForegroundColor Green
    }
    else {
        Write-Host "[FAIL] Missing required cook root: $Required" -ForegroundColor Red
        $Failures += "missing cook root $Required"
    }
}

if ($AlwaysCook -contains "/Game/MotoInteractionAnims") {
    Write-Host "[FAIL] MotoInteractionAnims is force-cooked wholesale; use scoped runtime roots instead." -ForegroundColor Red
    $Failures += "MotoInteractionAnims wholesale cook"
}
else {
    Write-Host "[PASS] MotoInteractionAnims cook scope is narrowed" -ForegroundColor Green
}

if ($NeverCook -contains "/Game/MotoInteractionAnims/Demo/Characters") {
    Write-Host "[FAIL] MotoInteraction demo characters are NeverCook, but runtime animations depend on their skeleton." -ForegroundColor Red
    $Failures += "MotoInteraction skeleton dependency blocked"
}
else {
    Write-Host "[PASS] MotoInteraction animation skeleton dependencies are cookable" -ForegroundColor Green
}

# A NeverCook root may not equal, contain, or sit inside an AlwaysCook root.
# Any such overlap is ambiguous and can turn a dependency into a late cook error.
foreach ($Always in $AlwaysCook) {
    foreach ($Never in $NeverCook) {
        $AlwaysPrefix = $Always + "/"
        $NeverPrefix = $Never + "/"
        $Overlaps =
            $Always.Equals($Never, [System.StringComparison]::OrdinalIgnoreCase) -or
            $Always.StartsWith($NeverPrefix, [System.StringComparison]::OrdinalIgnoreCase) -or
            $Never.StartsWith($AlwaysPrefix, [System.StringComparison]::OrdinalIgnoreCase)

        if ($Overlaps) {
            Write-Host "[FAIL] Conflicting cook roots: AlwaysCook=$Always NeverCook=$Never" -ForegroundColor Red
            $Failures += "overlapping cook roots"
        }
    }
}

if ($Failures.Count -gt 0) {
    Write-Host ""
    Write-Host "Cook contract verification FAILED." -ForegroundColor Red
    $Failures | Select-Object -Unique | ForEach-Object { Write-Host " - $_" -ForegroundColor Red }
    exit 2
}

Write-Host ""
Write-Host "Cook contract verification PASSED." -ForegroundColor Green
Write-Host "MotoInteraction runtime cook roots: Animations + Demo/Bike/Mesh" -ForegroundColor DarkGray
Write-Host "Dependency skeletons/materials: resolved transitively, not blacklisted" -ForegroundColor DarkGray
exit 0
