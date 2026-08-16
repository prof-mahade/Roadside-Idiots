param(
    [string]$PackagePath = "",
    [string]$ArchiveRoot = "C:\GameDev\RoadsideIdiots_Packaged",
    [switch]$Launch
)

$ErrorActionPreference = "Stop"

function Fail([string]$Message) {
    throw "DEMO 1 VERIFY FAILED: $Message"
}

if ([string]::IsNullOrWhiteSpace($PackagePath)) {
    if (-not (Test-Path $ArchiveRoot)) {
        Fail "Archive root does not exist: $ArchiveRoot"
    }

    $Latest = Get-ChildItem -Path $ArchiveRoot -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -like "RoadsideIdiots_Demo1_*" } |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1

    if (-not $Latest) {
        Fail "No RoadsideIdiots_Demo1_* package was found under $ArchiveRoot"
    }

    $PackagePath = $Latest.FullName
}

$PackagePath = (Resolve-Path $PackagePath).Path
$Exe = Get-ChildItem -Path $PackagePath -Recurse -File -Filter "RoadsideIdiots.exe" -ErrorAction SilentlyContinue |
    Select-Object -First 1
if (-not $Exe) {
    Fail "RoadsideIdiots.exe was not found under $PackagePath"
}

$Containers = @(Get-ChildItem -Path $PackagePath -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -in @(".pak", ".utoc", ".ucas") })
if ($Containers.Count -eq 0) {
    Fail "No cooked .pak/.utoc/.ucas files were found."
}

$Prereq = Get-ChildItem -Path $PackagePath -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match "UEPrereq.*\.exe" } |
    Select-Object -First 1

$Manifest = Join-Path $PackagePath "DEMO1_BUILD_INFO.txt"
$Readme = Join-Path $PackagePath "README_ROADSIDE_IDIOTS.txt"
$TestPlan = Join-Path $PackagePath "PLAYER_TEST_PLAN.md"
$ZipPath = "$PackagePath.zip"
$ChecksumPath = "$ZipPath.sha256.txt"

if (-not (Test-Path $Manifest)) {
    Fail "DEMO1_BUILD_INFO.txt is missing from the package root."
}
if (-not (Test-Path $Readme)) {
    Fail "README_ROADSIDE_IDIOTS.txt is missing from the package root."
}
if (-not (Test-Path $TestPlan)) {
    Fail "PLAYER_TEST_PLAN.md is missing from the package root."
}
if (-not (Test-Path $ZipPath)) {
    Fail "The shareable ZIP was not found: $ZipPath"
}
if (-not (Test-Path $ChecksumPath)) {
    Fail "The ZIP SHA-256 checksum file was not found: $ChecksumPath"
}

$ManifestText = Get-Content $Manifest -Raw
if ($ManifestText -notmatch 'Input contract preflight:\s*PASSED') {
    Fail "Build manifest does not record a passed input-contract preflight."
}

$ReadmeText = Get-Content $Readme -Raw
$RequiredReadmePhrases = @(
    'Y              quick race again after finish',
    'B              throw rotten egg / menu back / resume from Pause',
    'Menu / Start   pause / resume'
)
foreach ($Phrase in $RequiredReadmePhrases) {
    if ($ReadmeText -notlike "*$Phrase*") {
        Fail "Packaged README is missing current controller guidance: $Phrase"
    }
}

$ZipInfo = Get-Item $ZipPath
if ($ZipInfo.Length -le 0) {
    Fail "The shareable ZIP exists but is empty: $ZipPath"
}

$ExpectedHashLine = (Get-Content $ChecksumPath | Select-Object -First 1).Trim()
$ExpectedHash = ($ExpectedHashLine -split '\s+')[0].ToLowerInvariant()
$ActualHash = (Get-FileHash -Path $ZipPath -Algorithm SHA256).Hash.ToLowerInvariant()
if ([string]::IsNullOrWhiteSpace($ExpectedHash) -or $ExpectedHash -ne $ActualHash) {
    Fail "ZIP checksum mismatch. Expected $ExpectedHash but calculated $ActualHash"
}

Write-Host ""
Write-Host "ROADSIDE IDIOTS - DEMO 1 PACKAGE VERIFY" -ForegroundColor Cyan
Write-Host "Package       : $PackagePath"
Write-Host "Executable    : $($Exe.FullName)" -ForegroundColor Green
Write-Host "Cooked files  : $($Containers.Count)" -ForegroundColor Green
Write-Host "Prerequisite  : $(if ($Prereq) { $Prereq.FullName } else { 'not found / may be bundled differently' })"
Write-Host "Manifest      : $Manifest" -ForegroundColor Green
Write-Host "Player README : $Readme" -ForegroundColor Green
Write-Host "Test plan     : $TestPlan" -ForegroundColor Green
Write-Host "Share ZIP     : $ZipPath ($([math]::Round($ZipInfo.Length / 1MB, 1)) MB)" -ForegroundColor Green
Write-Host "SHA-256       : $ActualHash" -ForegroundColor Green
Write-Host "Input contract: recorded PASSED" -ForegroundColor Green
Write-Host ""

Write-Host "----- BUILD INFO -----" -ForegroundColor DarkCyan
Get-Content $Manifest | ForEach-Object { Write-Host $_ }
Write-Host "----------------------" -ForegroundColor DarkCyan
Write-Host ""

Write-Host "STATIC PACKAGE CHECK: PASSED" -ForegroundColor Green
Write-Host ""
Write-Host "PLAYER-TEST SMOKE TEST" -ForegroundColor Yellow
Write-Host "  1. QUICK RACE menu opens without stale VPR/build text."
Write-Host "  2. Keyboard: arrows navigate, Enter confirms, P/Esc pauses."
Write-Host "  3. Controller: D-pad navigates, A confirms, B backs/resumes, Menu/Start pauses."
Write-Host "  4. During gameplay confirm A=peel, B=egg, X=recover, LB/RB=slap."
Write-Host "  5. Press Y DURING an unfinished race: it must NOT reload the map."
Write-Host "  6. Finish a race; Enter/A/Y must restart the same configured race."
Write-Host "  7. Drive directly across visible repair patches/skid marks: zero physical bump."
Write-Host "  8. Run CLEAN: Opponents 6 | Laps 2 | Traffic 0."
Write-Host "     - stable AI completes clean laps without wall oscillation"
Write-Host "     - deliberate chaos is relatively uncommon"
Write-Host "  9. Run MAYHEM: Opponents 6 | Laps 2 | Traffic 6."
Write-Host "     - more rival incidents are noticeable without constant brawling"
Write-Host "     - traffic contact does not restore wall ping-pong"
Write-Host " 10. Confirm free PN vegetation, facade details, traffic shell and road wear render."
Write-Host " 11. Finish celebration produces no presentation physics/collision warning spam."
Write-Host " 12. Open README_ROADSIDE_IDIOTS.txt and PLAYER_TEST_PLAN.md before sharing."
Write-Host ""

if ($Launch) {
    Write-Host "Launching packaged Demo 1..." -ForegroundColor Cyan
    Start-Process -FilePath $Exe.FullName -WorkingDirectory $Exe.DirectoryName -ArgumentList @("-log", "-windowed", "-ResX=1280", "-ResY=720")
}
else {
    Write-Host "To launch automatically:" -ForegroundColor DarkGray
    Write-Host "  .\tools\verify_demo1_package.ps1 -PackagePath `"$PackagePath`" -Launch" -ForegroundColor DarkGray
}
