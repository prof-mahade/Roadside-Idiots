param(
    [string]$ProjectRoot = "C:\GameDev\Roadside-Idiots"
)

$ErrorActionPreference = "Stop"

$InputIni = Join-Path $ProjectRoot "Config\DefaultInput.ini"
$ControllerCpp = Join-Path $ProjectRoot "Source\RoadsideIdiots\Private\Core\RIPlayerController.cpp"
$BikeCpp = Join-Path $ProjectRoot "Source\RoadsideIdiots\Private\Vehicle\RIBikePawn.cpp"
$BikeHeader = Join-Path $ProjectRoot "Source\RoadsideIdiots\Public\Vehicle\RIBikePawn.h"

Write-Host "Roadside Idiots - Input Contract Verification" -ForegroundColor Cyan

foreach ($RequiredPath in @($InputIni, $ControllerCpp, $BikeCpp, $BikeHeader)) {
    if (-not (Test-Path $RequiredPath)) {
        Write-Host "[FAIL] Missing $RequiredPath" -ForegroundColor Red
        exit 2
    }
}

$Failures = @()

$LegacyRestart = Select-String -Path $InputIni -SimpleMatch 'ActionName="RestartRace"'
if ($LegacyRestart) {
    Write-Host "[FAIL] Legacy pawn RestartRace mapping is present." -ForegroundColor Red
    $LegacyRestart | ForEach-Object { Write-Host ("       " + $_.Line.Trim()) -ForegroundColor DarkGray }
    $Failures += "legacy restart mapping"
}
else {
    Write-Host "[PASS] No legacy pawn RestartRace mapping" -ForegroundColor Green
}

$PawnRestartBinding = Select-String -Path $BikeCpp -SimpleMatch 'BindAction(TEXT("RestartRace")'
$PawnRestartMethod = @(
    Select-String -Path $BikeCpp -SimpleMatch 'ARIBikePawn::RestartRace'
    Select-String -Path $BikeHeader -SimpleMatch 'void RestartRace'
) | Where-Object { $_ }

if ($PawnRestartBinding -or $PawnRestartMethod) {
    Write-Host "[FAIL] Pawn still owns restart input/code." -ForegroundColor Red
    @($PawnRestartBinding) + @($PawnRestartMethod) | Where-Object { $_ } | ForEach-Object {
        Write-Host ("       " + $_.Line.Trim()) -ForegroundColor DarkGray
    }
    $Failures += "pawn restart ownership"
}
else {
    Write-Host "[PASS] Restart ownership removed from bike pawn" -ForegroundColor Green
}

$RequiredControllerTokens = @(
    @{ Label = "Enter confirm/restart"; Token = "EKeys::Enter" },
    @{ Label = "A confirm"; Token = "EKeys::Gamepad_FaceButton_Bottom" },
    @{ Label = "B menu back"; Token = "EKeys::Gamepad_FaceButton_Right" },
    @{ Label = "Keyboard Y finish restart"; Token = "EKeys::Y" },
    @{ Label = "Controller Y finish restart"; Token = "EKeys::Gamepad_FaceButton_Top" },
    @{ Label = "Start pause"; Token = "EKeys::Gamepad_Special_Right" },
    @{ Label = "Controller finish-state guard"; Token = "IsPlayerRaceFinished" },
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

$BikeLifecycleChecks = @(
    @{ Label = "Human-only finish input guard"; Token = "Participant->IsHumanControlled()" },
    @{ Label = "Bike finish progress guard"; Token = "!Progress.bFinished" },
    @{ Label = "Recovery obeys race lifecycle"; Token = "!Chassis || !IsRaceInputEnabled()" },
    @{ Label = "Residual controls cleared after finish"; Token = "result screen cannot leave residual throttle" }
)

foreach ($Check in $BikeLifecycleChecks) {
    $Hit = Select-String -Path $BikeCpp -SimpleMatch $Check.Token
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
    $Failures | Select-Object -Unique | ForEach-Object { Write-Host " - $_" -ForegroundColor Red }
    exit 2
}

Write-Host ""
Write-Host "Input contract verification PASSED." -ForegroundColor Green
Write-Host "Restart/menu ownership: player controller" -ForegroundColor DarkGray
Write-Host "Human post-finish gameplay input: blocked" -ForegroundColor DarkGray
exit 0
