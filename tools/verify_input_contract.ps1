param(
    [string]$ProjectRoot = "C:\GameDev\Roadside-Idiots"
)

$ErrorActionPreference = "Stop"

$InputIni = Join-Path $ProjectRoot "Config\DefaultInput.ini"
$ControllerCpp = Join-Path $ProjectRoot "Source\RoadsideIdiots\Private\Core\RIPlayerController.cpp"

Write-Host "Roadside Idiots - Input Contract Verification" -ForegroundColor Cyan

if (-not (Test-Path $InputIni)) {
    Write-Host "[FAIL] Missing $InputIni" -ForegroundColor Red
    exit 2
}
if (-not (Test-Path $ControllerCpp)) {
    Write-Host "[FAIL] Missing $ControllerCpp" -ForegroundColor Red
    exit 2
}

$Failures = @()

$LegacyRestart = Select-String -Path $InputIni -SimpleMatch 'ActionName="RestartRace"'.Replace('\','')
if ($LegacyRestart) {
    Write-Host "[FAIL] Legacy pawn RestartRace mapping is present." -ForegroundColor Red
    $LegacyRestart | ForEach-Object { Write-Host ("       " + $_.Line.Trim()) -ForegroundColor DarkGray }
    $Failures += "legacy restart mapping"
}
else {
    Write-Host "[PASS] No legacy pawn RestartRace mapping" -ForegroundColor Green
}

$RequiredControllerTokens = @(
    @{ Label = "Enter confirm/restart"; Token = "EKeys::Enter" },
    @{ Label = "A confirm"; Token = "EKeys::Gamepad_FaceButton_Bottom" },
    @{ Label = "B menu back"; Token = "EKeys::Gamepad_FaceButton_Right" },
    @{ Label = "Keyboard Y finish restart"; Token = "EKeys::Y" },
    @{ Label = "Controller Y finish restart"; Token = "EKeys::Gamepad_FaceButton_Top" },
    @{ Label = "Start pause"; Token = "EKeys::Gamepad_Special_Right" },
    @{ Label = "Finish-state guard"; Token = "IsPlayerRaceFinished" },
    @{ Label = "Configured-race restart"; Token = "RestartConfiguredRace" }
)

foreach ($Check in $RequiredControllerTokens) {
    $Hit = Select-String -Path $ControllerCpp -SimpleMatch $Check.Token
    if ($Hit) {
        Write-Host ("[PASS] {0}" -f $Check.Label) -ForegroundColor Green
    }
    else {
        Write-Host ("[FAIL] {0} - token missing: {1}" -f $Check.Label, $Check.Token) -ForegroundColor Red
        $Failures += $Check.Label
    }
}

if ($Failures.Count -gt 0) {
    Write-Host ""
    Write-Host "Input contract verification FAILED." -ForegroundColor Red
    exit 2
}

Write-Host ""
Write-Host "Input contract verification PASSED." -ForegroundColor Green
Write-Host "Restart/menu ownership: player controller" -ForegroundColor DarkGray
exit 0
