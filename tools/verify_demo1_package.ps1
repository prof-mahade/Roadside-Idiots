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

Write-Host ""
Write-Host "ROADSIDE IDIOTS - DEMO 1 PACKAGE VERIFY" -ForegroundColor Cyan
Write-Host "Package       : $PackagePath"
Write-Host "Executable    : $($Exe.FullName)" -ForegroundColor Green
Write-Host "Cooked files  : $($Containers.Count)" -ForegroundColor Green
Write-Host "Prerequisite  : $(if ($Prereq) { $Prereq.FullName } else { 'not found / may be bundled differently' })"
Write-Host "Manifest      : $(if (Test-Path $Manifest) { $Manifest } else { 'not found' })"
Write-Host ""

if (Test-Path $Manifest) {
    Write-Host "----- BUILD INFO -----" -ForegroundColor DarkCyan
    Get-Content $Manifest | ForEach-Object { Write-Host $_ }
    Write-Host "----------------------" -ForegroundColor DarkCyan
    Write-Host ""
}

Write-Host "STATIC PACKAGE CHECK: PASSED" -ForegroundColor Green
Write-Host ""
Write-Host "MANUAL PLAY SMOKE TEST" -ForegroundColor Yellow
Write-Host "  1. Title/setup screen opens correctly."
Write-Host "  2. Run: Opponents 6 | Laps 2 | Traffic 0."
Write-Host "     - countdown/input lock works"
Write-Host "     - AI completes clean laps without wall oscillation"
Write-Host "     - lap/place/minimap/finish work"
Write-Host "  3. Run: Opponents 6 | Laps 2 | Traffic 6."
Write-Host "     - rivals attempt to pass/slow for traffic"
Write-Host "     - collision recovery works"
Write-Host "     - traffic contact does not cause persistent wall ping-pong"
Write-Host "  4. Test Q/E slap, F peel, G egg, R recover, P pause, Enter restart."
Write-Host "  5. Confirm banana/egg/poop effects, sound, HUD and free vegetation render."
Write-Host "  6. Finish a race and start another without returning to the editor."
Write-Host "  7. Quit normally from the game menu."
Write-Host ""

if ($Launch) {
    Write-Host "Launching packaged Demo 1..." -ForegroundColor Cyan
    Start-Process -FilePath $Exe.FullName -WorkingDirectory $Exe.DirectoryName -ArgumentList @("-log", "-windowed", "-ResX=1280", "-ResY=720")
}
else {
    Write-Host "To launch automatically:" -ForegroundColor DarkGray
    Write-Host "  .\tools\verify_demo1_package.ps1 -PackagePath `"$PackagePath`" -Launch" -ForegroundColor DarkGray
}
