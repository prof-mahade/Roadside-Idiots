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
$ZipPath = "$PackagePath.zip"

if (-not (Test-Path $Manifest)) {
    Fail "DEMO1_BUILD_INFO.txt is missing from the package root."
}
if (-not (Test-Path $Readme)) {
    Fail "README_ROADSIDE_IDIOTS.txt is missing from the package root."
}
if (-not (Test-Path $ZipPath)) {
    Fail "The shareable ZIP was not found: $ZipPath"
}

$ZipInfo = Get-Item $ZipPath
if ($ZipInfo.Length -le 0) {
    Fail "The shareable ZIP exists but is empty: $ZipPath"
}

Write-Host ""
Write-Host "ROADSIDE IDIOTS - DEMO 1 PACKAGE VERIFY" -ForegroundColor Cyan
Write-Host "Package       : $PackagePath"
Write-Host "Executable    : $($Exe.FullName)" -ForegroundColor Green
Write-Host "Cooked files  : $($Containers.Count)" -ForegroundColor Green
Write-Host "Prerequisite  : $(if ($Prereq) { $Prereq.FullName } else { 'not found / may be bundled differently' })"
Write-Host "Manifest      : $Manifest" -ForegroundColor Green
Write-Host "Player README : $Readme" -ForegroundColor Green
Write-Host "Share ZIP     : $ZipPath ($([math]::Round($ZipInfo.Length / 1MB, 1)) MB)" -ForegroundColor Green
Write-Host ""

Write-Host "----- BUILD INFO -----" -ForegroundColor DarkCyan
Get-Content $Manifest | ForEach-Object { Write-Host $_ }
Write-Host "----------------------" -ForegroundColor DarkCyan
Write-Host ""

Write-Host "STATIC PACKAGE CHECK: PASSED" -ForegroundColor Green
Write-Host ""
Write-Host "POLISH SMOKE TEST" -ForegroundColor Yellow
Write-Host "  1. QUICK RACE menu opens without stale VPR/build text."
Write-Host "  2. Opponents, laps, traffic and RACE CHAOS can be changed."
Write-Host "  3. SETTINGS contains Graphics, VSync, Steering Feel and Back."
Write-Host "  4. Run CLEAN: Opponents 6 | Laps 2 | Traffic 0."
Write-Host "     - stable AI completes clean laps without wall oscillation"
Write-Host "     - deliberate chaos is relatively uncommon"
Write-Host "     - opening control hint clears after the first part of the race"
Write-Host "  5. Run MAYHEM: Opponents 6 | Laps 2 | Traffic 6."
Write-Host "     - more rival incidents are noticeable without constant brawling"
Write-Host "     - traffic contact does not restore wall ping-pong"
Write-Host "     - finish/final-lap/minimap/HUD remain readable"
Write-Host "  6. Test Q/E slap, F peel, G egg, R recover, P/Esc pause, Enter race again."
Write-Host "  7. If available, test RT/LT/Left Stick, LB/RB, A/B/X/Y, D-pad and Menu/Start."
Write-Host "  8. Confirm banana/egg/poop effects, sound, bike/rider and free vegetation render."
Write-Host "  9. Finish, race again, return to setup and quit normally."
Write-Host " 10. Open README_ROADSIDE_IDIOTS.txt and confirm the shared controls match the build."
Write-Host ""

if ($Launch) {
    Write-Host "Launching packaged Demo 1..." -ForegroundColor Cyan
    Start-Process -FilePath $Exe.FullName -WorkingDirectory $Exe.DirectoryName -ArgumentList @("-log", "-windowed", "-ResX=1280", "-ResY=720")
}
else {
    Write-Host "To launch automatically:" -ForegroundColor DarkGray
    Write-Host "  .\tools\verify_demo1_package.ps1 -PackagePath `"$PackagePath`" -Launch" -ForegroundColor DarkGray
}
