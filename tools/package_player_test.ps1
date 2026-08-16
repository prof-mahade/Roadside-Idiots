param(
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8",
    [string]$ArchiveRoot = "C:\GameDev\RoadsideIdiots_Packaged",
    [ValidateSet("Development", "Shipping")]
    [string]$Configuration = "Shipping"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$BasePackager = Join-Path $PSScriptRoot "package_demo1.ps1"
$BugfixVerifier = Join-Path $PSScriptRoot "verify_bugfix_contracts.ps1"
$PackageVerifier = Join-Path $PSScriptRoot "verify_demo1_package.ps1"
$ContentDir = Join-Path $RepoRoot "Content"

function Fail([string]$Message) {
    throw "PLAYER TEST PACKAGE FAILED: $Message"
}

foreach ($RequiredFile in @($BasePackager, $BugfixVerifier, $PackageVerifier)) {
    if (-not (Test-Path $RequiredFile)) {
        Fail "required tool is missing: $RequiredFile"
    }
}

Write-Host "Roadside Idiots - Standalone Player-Test Pipeline" -ForegroundColor Cyan
Write-Host ""

# A distributable build must correspond to a reproducible tracked source state.
# Untracked imported Content is intentionally allowed because approved Fab/free
# binary assets are local and are not stored in Git. Tracked modifications are
# not allowed because they would make the package differ from the recorded SHA.
$TrackedChanges = @(& git -C $RepoRoot status --porcelain --untracked-files=no 2>$null)
if ($LASTEXITCODE -ne 0) {
    Fail "could not inspect tracked Git working-tree state."
}

if ($TrackedChanges.Count -gt 0) {
    Write-Host "[FAIL] Tracked working-tree changes must be stashed/committed before a player-test package:" -ForegroundColor Red
    $TrackedChanges | ForEach-Object { Write-Host "       $_" -ForegroundColor Red }
    Write-Host ""
    Write-Host "Untracked local Content is allowed; do not delete it." -ForegroundColor Yellow
    Fail "tracked working tree is not clean."
}
Write-Host "[PASS] Tracked source/config/tooling tree is clean" -ForegroundColor Green

$Branch = (& git -C $RepoRoot rev-parse --abbrev-ref HEAD 2>$null).Trim()
$FullCommit = (& git -C $RepoRoot rev-parse HEAD 2>$null).Trim()
$ShortCommit = (& git -C $RepoRoot rev-parse --short HEAD 2>$null).Trim()
if ([string]::IsNullOrWhiteSpace($FullCommit)) {
    Fail "could not resolve current Git commit."
}

if ($Branch -ne "dev/mvp-foundation" -and $Branch -ne "main") {
    Write-Warning "Packaging from branch '$Branch'. Expected dev/mvp-foundation or main."
}

Write-Host "[PASS] Source fingerprint: $Branch @ $ShortCommit" -ForegroundColor Green

# The current world layer requires these approved free local presentation assets.
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
    Write-Host "[FAIL] Approved free assets required by the current presentation are missing:" -ForegroundColor Red
    $MissingApprovedAssets | ForEach-Object { Write-Host "       $_" -ForegroundColor Red }
    Fail "required approved free assets are missing."
}
Write-Host "[PASS] Approved free vegetation assets are present (4/4)" -ForegroundColor Green

Write-Host ""
Write-Host "Running combined input/audio/lifecycle preflight..." -ForegroundColor Cyan
$PreflightLines = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $BugfixVerifier -ProjectRoot $RepoRoot 2>&1)
$PreflightExit = $LASTEXITCODE
$PreflightLines | ForEach-Object { Write-Host $_ }
if ($PreflightExit -ne 0) {
    Fail "bugfix contract preflight failed."
}

$BeforePackages = @{}
if (Test-Path $ArchiveRoot) {
    Get-ChildItem -Path $ArchiveRoot -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -like "RoadsideIdiots_Demo1_*" } |
        ForEach-Object { $BeforePackages[$_.FullName] = $true }
}

Write-Host ""
Write-Host "Building standalone $Configuration package..." -ForegroundColor Cyan
& $BasePackager -EngineRoot $EngineRoot -ArchiveRoot $ArchiveRoot -Configuration $Configuration

$NewPackages = @(Get-ChildItem -Path $ArchiveRoot -Directory -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -like "RoadsideIdiots_Demo1_*" -and -not $BeforePackages.ContainsKey($_.FullName) } |
    Sort-Object LastWriteTime -Descending)

if ($NewPackages.Count -eq 0) {
    Fail "base packaging completed but no new package directory could be identified."
}

$Package = $NewPackages[0]
$PackagePath = $Package.FullName

$EvidencePath = Join-Path $PackagePath "PLAYER_TEST_BUILD_INFO.txt"
@"
ROADSIDE IDIOTS - PLAYER TEST BUILD EVIDENCE
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Branch: $Branch
Git commit (full): $FullCommit
Git commit (short): $ShortCommit
Configuration: $Configuration
Tracked working tree clean before packaging: YES
Approved free vegetation assets present before packaging: 4/4
Combined input/audio/lifecycle preflight: PASSED
Packaging pipeline: tools/package_player_test.ps1

RUNTIME GATE NOTES
- Shipping logging is not assumed by this pipeline.
- Audio continuity, Y finish restart, finish-state input lock, driving stability,
  and presentation quality must be checked in the packaged executable by a human.
- Do not use Editor-only log success as a substitute for the standalone smoke test.
"@ | Set-Content -Path $EvidencePath -Encoding UTF8

$PreflightEvidencePath = Join-Path $PackagePath "BUGFIX_PREFLIGHT.txt"
$PreflightLines | Set-Content -Path $PreflightEvidencePath -Encoding UTF8

# The base packager creates its ZIP before the extra player-test evidence above.
# Rebuild the ZIP/checksum so the distributable contains the exact evidence that
# the verifier and outside-test workflow rely on.
$ZipPath = "$PackagePath.zip"
$ChecksumPath = "$ZipPath.sha256.txt"
if (Test-Path $ZipPath) { Remove-Item $ZipPath -Force }
if (Test-Path $ChecksumPath) { Remove-Item $ChecksumPath -Force }

Write-Host ""
Write-Host "Rebuilding player-test ZIP with reproducibility evidence..." -ForegroundColor Cyan
Compress-Archive -Path (Join-Path $PackagePath "*") -DestinationPath $ZipPath -CompressionLevel Optimal
if (-not (Test-Path $ZipPath)) {
    Fail "shareable ZIP was not created: $ZipPath"
}

$ZipHash = Get-FileHash -Path $ZipPath -Algorithm SHA256
"$($ZipHash.Hash.ToLowerInvariant())  $([System.IO.Path]::GetFileName($ZipPath))" |
    Set-Content -Path $ChecksumPath -Encoding ASCII

Write-Host ""
Write-Host "Running static package/ZIP verification..." -ForegroundColor Cyan
& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $PackageVerifier -PackagePath $PackagePath
if ($LASTEXITCODE -ne 0) {
    Fail "post-package verification failed."
}

Write-Host ""
Write-Host "PLAYER TEST PACKAGE READY" -ForegroundColor Green
Write-Host "Package : $PackagePath"
Write-Host "ZIP     : $ZipPath" -ForegroundColor Green
Write-Host "SHA-256 : $($ZipHash.Hash.ToLowerInvariant())" -ForegroundColor Green
Write-Host "Source  : $Branch @ $ShortCommit"
Write-Host ""
Write-Host "Next: launch the packaged executable and perform the human smoke gate in PLAYER_TEST_PLAN.md." -ForegroundColor Yellow
