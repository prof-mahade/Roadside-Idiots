param(
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8",
    [string]$ArchiveRoot = "C:\GameDev\RoadsideIdiots_Packaged",
    [ValidateSet("Development", "Shipping")]
    [string]$Configuration = "Shipping"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $RepoRoot "RoadsideIdiots.uproject"
$RunUAT = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"
$ContentDir = Join-Path $RepoRoot "Content"
$SourceDir = Join-Path $RepoRoot "Source"
$ConfigDir = Join-Path $RepoRoot "Config"
$DefaultGameIni = Join-Path $ConfigDir "DefaultGame.ini"

function Fail([string]$Message) {
    throw "DEMO 1 PREFLIGHT FAILED: $Message"
}

if (-not (Test-Path $Project)) {
    Fail "RoadsideIdiots.uproject was not found at: $Project"
}

if (-not (Test-Path $RunUAT)) {
    Fail "RunUAT.bat was not found. Expected UE 5.8 at: $EngineRoot"
}

$ProjectVersion = "unknown"
if (Test-Path $DefaultGameIni) {
    $VersionLine = Get-Content $DefaultGameIni |
        Where-Object { $_ -match '^ProjectVersion=' } |
        Select-Object -First 1
    if ($VersionLine) {
        $ProjectVersion = ($VersionLine -replace '^ProjectVersion=', '').Trim()
    }
}
$VersionTag = if ([string]::IsNullOrWhiteSpace($ProjectVersion)) { "unknown" } else { $ProjectVersion -replace '[^A-Za-z0-9._-]', '_' }

if (-not (Test-Path $ContentDir)) {
    Write-Warning "Content folder is missing. Imported local presentation assets will not be available to the cook."
}
else {
    # PERMANENT PROJECT RULE: free/custom content only.
    # Search recursively, not just top-level folders. The paid/licensing-risk
    # SankoolArts pack was removed at the user's request and must never enter a
    # distributable package again.
    $ForbiddenContent = Get-ChildItem -Path $ContentDir -Recurse -Force -ErrorAction SilentlyContinue |
        Where-Object {
            $_.FullName -match "(?i)Sankool" -or
            $_.FullName -match "(?i)CompoundWall_Kit"
        }

    if ($ForbiddenContent) {
        $Names = ($ForbiddenContent | Select-Object -First 12 | ForEach-Object { $_.FullName }) -join "`n  "
        Fail "forbidden licensing-risk content was found:`n  $Names"
    }
}

# Also prevent a source/config soft reference from silently cooking a removed
# paid asset through another dependency.
$ReferenceRoots = @($SourceDir, $ConfigDir) | Where-Object { Test-Path $_ }
if ($ReferenceRoots.Count -gt 0) {
    $ForbiddenReferences = Get-ChildItem -Path $ReferenceRoots -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Extension -in @(".cpp", ".h", ".ini", ".cs") } |
        Select-String -Pattern "Sankool|CompoundWall_Kit" -CaseSensitive:$false -ErrorAction SilentlyContinue

    if ($ForbiddenReferences) {
        $Refs = ($ForbiddenReferences | Select-Object -First 12 | ForEach-Object {
            "$($_.Path):$($_.LineNumber): $($_.Line.Trim())"
        }) -join "`n  "
        Fail "source/config still references forbidden content:`n  $Refs"
    }
}

$ExpectedFreeAssets = @(
    "PN_Banana\Meshes\plants\banana_01_07.uasset",
    "PN_Banana\Meshes\plants\banana_02_05.uasset",
    "PN_tropicalGroundPlants\Meshes\tropicalPlant_01_04.uasset",
    "PN_tropicalGroundPlants\Meshes\tropicalPlant_05_04.uasset"
)

$MissingApprovedAssets = @()
foreach ($RelativeAsset in $ExpectedFreeAssets) {
    $FullAssetPath = Join-Path $ContentDir $RelativeAsset
    if (-not (Test-Path $FullAssetPath)) {
        $MissingApprovedAssets += $RelativeAsset
        Write-Warning "Expected approved free asset is missing locally: $RelativeAsset"
    }
}

$GitCommit = "unknown"
try {
    $GitCommit = (& git -C $RepoRoot rev-parse --short HEAD 2>$null).Trim()
    if ([string]::IsNullOrWhiteSpace($GitCommit)) { $GitCommit = "unknown" }
}
catch {
    Write-Warning "Could not read Git commit. Package will still continue."
}

$GitStatus = @()
try {
    $GitStatus = @(& git -C $RepoRoot status --short 2>$null)
}
catch {}

if ($GitStatus.Count -gt 0) {
    Write-Warning "Local working tree has changes. This is allowed because imported Content is intentionally local, but the build manifest will record it as DIRTY."
}

$Stamp = Get-Date -Format "yyyyMMdd_HHmmss"
$ArchiveDir = Join-Path $ArchiveRoot "RoadsideIdiots_Demo1_${VersionTag}_${Configuration}_${GitCommit}_$Stamp"
New-Item -ItemType Directory -Force -Path $ArchiveDir | Out-Null

Write-Host ""
Write-Host "ROADSIDE IDIOTS - DEMO 1 WINDOWS PACKAGE" -ForegroundColor Cyan
Write-Host "Project       : $Project"
Write-Host "Version       : $ProjectVersion"
Write-Host "Engine        : $EngineRoot"
Write-Host "Configuration : $Configuration"
Write-Host "Git commit    : $GitCommit"
Write-Host "Output        : $ArchiveDir"
Write-Host "Free-only     : recursive content/source preflight PASSED" -ForegroundColor Green
Write-Host ""

& $RunUAT BuildCookRun `
    "-project=$Project" `
    -noP4 `
    -platform=Win64 `
    "-clientconfig=$Configuration" `
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
    Fail "Unreal BuildCookRun failed with exit code $LASTEXITCODE. Review the UAT error above before retrying."
}

$Exe = Get-ChildItem -Path $ArchiveDir -Recurse -File -Filter "RoadsideIdiots.exe" -ErrorAction SilentlyContinue |
    Select-Object -First 1
if (-not $Exe) {
    Fail "BuildCookRun reported success, but RoadsideIdiots.exe was not found under $ArchiveDir"
}

$CookedContainers = @(Get-ChildItem -Path $ArchiveDir -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -in @(".pak", ".utoc", ".ucas") })
if ($CookedContainers.Count -eq 0) {
    Fail "No cooked .pak/.utoc/.ucas container was found in the archive."
}

$ManifestPath = Join-Path $ArchiveDir "DEMO1_BUILD_INFO.txt"
$DirtyText = if ($GitStatus.Count -gt 0) { "YES" } else { "NO" }
$MissingText = if ($MissingApprovedAssets.Count -gt 0) { $MissingApprovedAssets -join ", " } else { "none" }

@"
ROADSIDE IDIOTS - DEMO 1 BUILD
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Project version: $ProjectVersion
Git commit: $GitCommit
Working tree dirty at package time: $DirtyText
Configuration: $Configuration
Engine: $EngineRoot
Executable: $($Exe.FullName)
Cooked containers: $($CookedContainers.Count)
Expected approved free assets missing: $MissingText

CONTENT POLICY FOR THIS PROJECT
- Free/custom content only.
- SankoolArts / CompoundWall_Kit content is forbidden and packaging preflight blocks it.
- Imported PN_Banana and PN_tropicalGroundPlants content is treated as approved free local presentation content for this project.
"@ | Set-Content -Path $ManifestPath -Encoding UTF8

$PlayerReadmePath = Join-Path $ArchiveDir "README_ROADSIDE_IDIOTS.txt"
@"
ROADSIDE IDIOTS - DEMO 1
The road is dangerous. The riders are worse.
Version: $ProjectVersion

QUICK START
1. Run Windows\RoadsideIdiots.exe
2. Choose opponents, laps, traffic and race chaos.
3. Select START RACE.

KEYBOARD
W              accelerate
S              brake / reverse
A / D          steer
Q / E          slap left / right
F              drop banana peel
G              throw rotten egg
R              recover bike
P / Esc        pause
Enter          confirm / race again
Arrow keys     menu navigation

GAMEPAD (Xbox-style layout)
RT             accelerate
LT             brake / reverse
Left Stick     steer
LB / RB        slap left / right
A              drop banana peel / menu confirm
B              throw rotten egg
X              recover bike
Y              race again
Menu / Start   pause
D-pad          menu navigation

RACE CHAOS
CLEAN          mostly racing, fewer deliberate rival incidents
BALANCED       intended mix of racing and petty chaos
MAYHEM         more frequent rival trouble; core driving AI is unchanged

SETTINGS
Graphics Quality    LOW / MEDIUM / HIGH / EPIC
VSync               ON / OFF
Steering Feel       CALM / NORMAL / QUICK
                    CALM gives finer center-stick control.
                    QUICK responds earlier to analog-stick movement.
                    Keyboard full-left/full-right is unchanged.

DEMO NOTES
- This is a solo prototype/demo build.
- Multiplayer, additional maps, final art and deeper progression are future work.
- If the game is blocked on first launch, Windows may ask for Unreal prerequisites; the package includes the standard prerequisite installer.

Build: $GitCommit ($Configuration)
"@ | Set-Content -Path $PlayerReadmePath -Encoding UTF8

$ZipPath = "$ArchiveDir.zip"
if (Test-Path $ZipPath) {
    Remove-Item $ZipPath -Force
}

Write-Host "Creating shareable ZIP..." -ForegroundColor Cyan
Compress-Archive -Path (Join-Path $ArchiveDir "*") -DestinationPath $ZipPath -CompressionLevel Optimal
if (-not (Test-Path $ZipPath)) {
    Fail "Package succeeded, but the shareable ZIP was not created: $ZipPath"
}

$ZipHash = Get-FileHash -Path $ZipPath -Algorithm SHA256
$ChecksumPath = "$ZipPath.sha256.txt"
"$($ZipHash.Hash.ToLowerInvariant())  $([System.IO.Path]::GetFileName($ZipPath))" |
    Set-Content -Path $ChecksumPath -Encoding ASCII

Write-Host ""
Write-Host "Demo 1 package completed successfully." -ForegroundColor Green
Write-Host "Version    : $ProjectVersion"
Write-Host "Executable : $($Exe.FullName)"
Write-Host "Manifest   : $ManifestPath"
Write-Host "Readme     : $PlayerReadmePath"
Write-Host "Package    : $ArchiveDir"
Write-Host "Share ZIP  : $ZipPath" -ForegroundColor Green
Write-Host "SHA-256    : $($ZipHash.Hash.ToLowerInvariant())" -ForegroundColor Green
Write-Host "Checksum   : $ChecksumPath"
Write-Host ""
Write-Host "NEXT: run tools\verify_demo1_package.ps1 against this folder, then perform the manual smoke test before sharing the ZIP." -ForegroundColor Yellow
