param(
    [Parameter(Mandatory = $true)]
    [string]$PackagePath
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$Verifier = Join-Path $PSScriptRoot "verify_demo1_package.ps1"

function Fail([string]$Message) {
    throw "PLAYER TEST FINALIZE FAILED: $Message"
}

if (-not (Test-Path $PackagePath)) {
    Fail "package directory does not exist: $PackagePath"
}
if (-not (Test-Path $Verifier)) {
    Fail "package verifier is missing: $Verifier"
}

$PackagePath = (Resolve-Path $PackagePath).Path
$Exe = Get-ChildItem -Path $PackagePath -Recurse -File -Filter "RoadsideIdiots.exe" -ErrorAction SilentlyContinue |
    Select-Object -First 1
if (-not $Exe) {
    Fail "RoadsideIdiots.exe is missing from the package."
}

$Containers = @(Get-ChildItem -Path $PackagePath -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -in @('.pak', '.utoc', '.ucas') })
if ($Containers.Count -eq 0) {
    Fail "cooked containers are missing; refusing to finalize an incomplete package."
}

Write-Host "Roadside Idiots - Existing Player-Test Package Finalizer" -ForegroundColor Cyan
Write-Host "Package : $PackagePath"
Write-Host "Exe     : $($Exe.FullName)"
Write-Host "Cooked  : $($Containers.Count) container file(s)"
Write-Host ""

$PdbFiles = @(Get-ChildItem -Path $PackagePath -Recurse -File -Filter "*.pdb" -ErrorAction SilentlyContinue)
$RemovedBytes = 0L
foreach ($Pdb in $PdbFiles) {
    $RemovedBytes += $Pdb.Length
    Write-Host "Removing debug symbol: $($Pdb.FullName)" -ForegroundColor Yellow
    Remove-Item -Path $Pdb.FullName -Force
}

if ($PdbFiles.Count -eq 0) {
    Write-Host "[PASS] No loose PDB files required removal" -ForegroundColor Green
}
else {
    Write-Host "[PASS] Removed $($PdbFiles.Count) PDB file(s), $([math]::Round($RemovedBytes / 1MB, 1)) MB before ZIP compression" -ForegroundColor Green
}

$FinalizeInfo = Join-Path $PackagePath "PLAYER_TEST_FINALIZATION.txt"
@"
ROADSIDE IDIOTS - PLAYER TEST FINALIZATION
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Existing cooked executable preserved: YES
Existing cooked container set preserved: YES
PDB files removed: $($PdbFiles.Count)
PDB bytes removed before ZIP compression: $RemovedBytes
Finalizer: tools/finalize_player_test_package.ps1
Reason: tester distributable hygiene; no recook/rebuild performed
"@ | Set-Content -Path $FinalizeInfo -Encoding UTF8

$ZipPath = "$PackagePath.zip"
$ChecksumPath = "$ZipPath.sha256.txt"
if (Test-Path $ZipPath) { Remove-Item -Path $ZipPath -Force }
if (Test-Path $ChecksumPath) { Remove-Item -Path $ChecksumPath -Force }

Write-Host "Rebuilding ZIP from the existing cooked package..." -ForegroundColor Cyan
Compress-Archive -Path (Join-Path $PackagePath "*") -DestinationPath $ZipPath -CompressionLevel Optimal
if (-not (Test-Path $ZipPath)) {
    Fail "ZIP recreation failed: $ZipPath"
}

$Hash = Get-FileHash -Path $ZipPath -Algorithm SHA256
"$($Hash.Hash.ToLowerInvariant())  $([System.IO.Path]::GetFileName($ZipPath))" |
    Set-Content -Path $ChecksumPath -Encoding ASCII

Write-Host "[PASS] ZIP rebuilt: $([math]::Round((Get-Item $ZipPath).Length / 1MB, 1)) MB" -ForegroundColor Green
Write-Host "[PASS] SHA-256: $($Hash.Hash.ToLowerInvariant())" -ForegroundColor Green
Write-Host ""
Write-Host "Running strict package verification..." -ForegroundColor Cyan
& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Verifier -PackagePath $PackagePath
if ($LASTEXITCODE -ne 0) {
    Fail "strict post-finalization verification failed."
}

Write-Host ""
Write-Host "PLAYER TEST PACKAGE FINALIZED - NO RECOOK REQUIRED" -ForegroundColor Green
Write-Host "Package : $PackagePath"
Write-Host "ZIP     : $ZipPath"
Write-Host "SHA-256 : $($Hash.Hash.ToLowerInvariant())"
Write-Host "Evidence: $FinalizeInfo"
