param(
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8",
    [string]$ArchiveRoot = "C:\GameDev\RoadsideIdiots_Packaged"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $RepoRoot "RoadsideIdiots.uproject"
$RunUAT = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"
$ContentDir = Join-Path $RepoRoot "Content"

if (-not (Test-Path $Project)) {
    throw "RoadsideIdiots.uproject was not found at: $Project"
}

if (-not (Test-Path $RunUAT)) {
    throw "RunUAT.bat was not found. Expected UE 5.8 at: $EngineRoot"
}

if (-not (Test-Path $ContentDir)) {
    Write-Warning "Content folder is missing. Local imported presentation assets will not be available to the cook."
}
else {
    # Permanent project rule: free/custom content only. The SankoolArts compound
    # pack was deliberately removed after the user's licensing/payment concern.
    $ForbiddenFolders = Get-ChildItem -Path $ContentDir -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match "Sankool" }

    if ($ForbiddenFolders) {
        $Names = ($ForbiddenFolders | ForEach-Object { $_.FullName }) -join ", "
        throw "Packaging stopped: removed licensing-risk SankoolArts content was found: $Names"
    }
}

$ExpectedFreeAssets = @(
    "PN_Banana\Meshes\plants\banana_01_07.uasset",
    "PN_Banana\Meshes\plants\banana_02_05.uasset",
    "PN_tropicalGroundPlants\Meshes\tropicalPlant_01_04.uasset",
    "PN_tropicalGroundPlants\Meshes\tropicalPlant_05_04.uasset"
)

foreach ($RelativeAsset in $ExpectedFreeAssets) {
    $FullAssetPath = Join-Path $ContentDir $RelativeAsset
    if (-not (Test-Path $FullAssetPath)) {
        Write-Warning "Expected approved free asset is missing locally: $RelativeAsset"
    }
}

$Stamp = Get-Date -Format "yyyyMMdd_HHmmss"
$ArchiveDir = Join-Path $ArchiveRoot "RoadsideIdiots_Demo1_$Stamp"
New-Item -ItemType Directory -Force -Path $ArchiveDir | Out-Null

Write-Host ""
Write-Host "ROADSIDE IDIOTS - DEMO 1 WINDOWS PACKAGE" -ForegroundColor Cyan
Write-Host "Project : $Project"
Write-Host "Engine  : $EngineRoot"
Write-Host "Output  : $ArchiveDir"
Write-Host ""

& $RunUAT BuildCookRun `
    "-project=$Project" `
    -noP4 `
    -platform=Win64 `
    -clientconfig=Development `
    -build `
    -cook `
    -stage `
    -pak `
    -compressed `
    -prereqs `
    -archive `
    "-archivedirectory=$ArchiveDir" `
    -utf8output

if ($LASTEXITCODE -ne 0) {
    throw "Demo packaging failed with exit code $LASTEXITCODE. Review the UAT error above before retrying."
}

Write-Host ""
Write-Host "Demo 1 package completed successfully." -ForegroundColor Green
Write-Host "Packaged files: $ArchiveDir"
