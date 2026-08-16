param(
    [string]$ProjectRoot = "C:\GameDev\Roadside-Idiots"
)

$ErrorActionPreference = "Stop"
$ToolsRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

$InputVerifier = Join-Path $ToolsRoot "verify_input_contract.ps1"
$AudioVerifier = Join-Path $ToolsRoot "verify_audio_contract.ps1"

Write-Host "Roadside Idiots - Bugfix Contract Preflight" -ForegroundColor Cyan
Write-Host ""

foreach ($Verifier in @($InputVerifier, $AudioVerifier)) {
    if (-not (Test-Path $Verifier)) {
        Write-Host "[FAIL] Missing verifier: $Verifier" -ForegroundColor Red
        exit 2
    }

    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Verifier -ProjectRoot $ProjectRoot
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "[FAIL] Contract preflight stopped at: $Verifier" -ForegroundColor Red
        exit $LASTEXITCODE
    }

    Write-Host ""
}

Write-Host "[PASS] BUGFIX CONTRACT PREFLIGHT" -ForegroundColor Green
Write-Host " - restart/menu ownership is single-owner" -ForegroundColor DarkGray
Write-Host " - human post-finish gameplay input is blocked" -ForegroundColor DarkGray
Write-Host " - engine audio is persistent and presentation-owned" -ForegroundColor DarkGray
Write-Host " - bike movement remains physics-only" -ForegroundColor DarkGray
exit 0
