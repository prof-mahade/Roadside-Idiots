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
$BugfixVerifier = Join-Path $PSScriptRoot "verify_bugfix_contracts.ps1"
$PlayerTestPlanSource = Join-Path $RepoRoot "docs\PLAYER_TEST_PLAN.md"

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
    Fail "Content folder is missing. Required approved local presentation assets are unavailable."
}

# PERMANENT PROJECT RULE: free/custom content only.
$ForbiddenContent = Get-ChildItem -Path $ContentDir -Recurse -Force -ErrorAction SilentlyContinue |
    Where-Object {
        $_.FullName -match "(?i)Sankool" -or
        $_.FullName -match "(?i)CompoundWall_Kit"
    }

if ($ForbiddenContent) {
    $Names = ($ForbiddenContent | Select-Object -First 12 | ForEach-Object { $_.FullName }) -join "`n  "
    Fail "forbidden licensing-risk content was found:`n  $Names"
}

# Prevent source/config soft references from silently cooking removed paid content.
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

# Functional preflight: single-owner input/restart, finish lifecycle, persistent engine.
if (-not (Test-Path $BugfixVerifier)) {
    Fail "combined bugfix contract verifier is missing: $BugfixVerifier"
}

& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $BugfixVerifier -ProjectRoot $RepoRoot
if ($LASTEXITCODE -ne 0) {
    Fail "combined input/audio/lifecycle contract verification failed."
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
    }
}
if ($MissingApprovedAssets.Count -gt 0) {
    $MissingApprovedAssets | ForEach-Object { Write-Host "       $_" -ForegroundColor Red }
    Fail "required approved free presentation assets are missing."
}

$GitCommit = "unknown"
try {
    $GitCommit = (& git -C $RepoRoot rev-parse --short HEAD 2>$null).Trim()
    if ([string]::IsNullOrWhiteSpace($GitCommit)) { $GitCommit = "unknown" }
}
catch {
    Write-Warning "Could not read Git commit. Package will still continue."
}

# Reproducibility is about tracked project state. Approved imported Content is
# intentionally local/untracked and must not make an otherwise reproducible build DIRTY.
$TrackedGitStatus = @()
try {
    $TrackedGitStatus = @(& git -C $RepoRoot status --short --untracked-files=no 2>$null)
}
catch {}

if ($TrackedGitStatus.Count -gt 0) {
    Write-Warning "Tracked working tree has changes; manifest will record DIRTY."
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
Write-Host "Bugfix contract: input/audio/lifecycle preflight PASSED" -ForegroundColor Green
Write-Host "Approved assets: required local free vegetation 4/4" -ForegroundColor Green
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

# Shipping testers do not need private debug symbols. Keep them in local build
# intermediates, but remove them from the distributable before ZIP/checksum.
$RemovedPdbCount = 0
if ($Configuration -eq "Shipping") {
    $PdbFiles = @(Get-ChildItem -Path $ArchiveDir -Recurse -File -Filter "*.pdb" -ErrorAction SilentlyContinue)
    foreach ($Pdb in $PdbFiles) {
        Remove-Item -Path $Pdb.FullName -Force
        ++$RemovedPdbCount
    }
}

$ManifestPath = Join-Path $ArchiveDir "DEMO1_BUILD_INFO.txt"
$DirtyText = if ($TrackedGitStatus.Count -gt 0) { "YES" } else { "NO" }

@"
ROADSIDE IDIOTS - DEMO 1 BUILD
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Project version: $ProjectVersion
Git commit: $GitCommit
Tracked working tree dirty at package time: $DirtyText
Local approved Content expected/present: YES
Configuration: $Configuration
Engine: $EngineRoot
Executable: $($Exe.FullName)
Cooked containers: $($CookedContainers.Count)
Required approved free assets present: 4/4
Combined input/audio/lifecycle preflight: PASSED
Input contract preflight: PASSED
Shipping PDB files removed from distributable: $RemovedPdbCount

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
P / Esc        pause / resume
Enter          menu confirm / race again after finish
Y              quick race again after finish
Arrow keys     menu navigation

GAMEPAD (Xbox-style layout)
RT             accelerate
LT             brake / reverse
Left Stick     steer
LB / RB        slap left / right
A              drop banana peel / menu confirm / race again after finish
B              throw rotten egg / menu back / resume from Pause
X              recover bike
Y              quick race again after finish
Menu / Start   pause / resume
D-pad          menu navigation

Y is intentionally a finish-only restart shortcut. Pressing Y during an unfinished race does not reload the map.

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
- If the game is blocked on first launch, Windows may ask for Unreal prerequisites; the package includes the standard prerequisite installer when generated by UAT.

Build: $GitCommit ($Configuration)
"@ | Set-Content -Path $PlayerReadmePath -Encoding UTF8

$PackagedTestPlan = Join-Path $ArchiveDir "PLAYER_TEST_PLAN.md"
if (Test-Path $PlayerTestPlanSource) {
    Copy-Item -Path $PlayerTestPlanSource -Destination $PackagedTestPlan -Force
}
else {
    Write-Warning "Player test plan was not found at $PlayerTestPlanSource"
}

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
if (Test-Path $PackagedTestPlan) {
    Write-Host "Test plan  : $PackagedTestPlan"
}
Write-Host "PDB removed: $RemovedPdbCount"
Write-Host "Package    : $ArchiveDir"
Write-Host "Share ZIP  : $ZipPath" -ForegroundColor Green
Write-Host "SHA-256    : $($ZipHash.Hash.ToLowerInvariant())" -ForegroundColor Green
