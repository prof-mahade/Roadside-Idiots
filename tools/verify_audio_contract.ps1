param(
    [string]$ProjectRoot = "C:\GameDev\Roadside-Idiots"
)

$ErrorActionPreference = "Stop"

$PresentationCpp = Join-Path $ProjectRoot "Source\RoadsideIdiots\Private\Presentation\RIPresentationWorldSubsystem.cpp"
$PresentationHeader = Join-Path $ProjectRoot "Source\RoadsideIdiots\Public\Presentation\RIPresentationWorldSubsystem.h"
$MovementCpp = Join-Path $ProjectRoot "Source\RoadsideIdiots\Private\Vehicle\RIBikeMovementComponent.cpp"
$AudioEventsCpp = Join-Path $ProjectRoot "Source\RoadsideIdiots\Private\Audio\RIAudioEvents.cpp"

Write-Host "Roadside Idiots - Audio Contract Verification" -ForegroundColor Cyan

foreach ($RequiredPath in @($PresentationCpp, $PresentationHeader, $MovementCpp, $AudioEventsCpp)) {
    if (-not (Test-Path $RequiredPath)) {
        Write-Host "[FAIL] Missing $RequiredPath" -ForegroundColor Red
        exit 2
    }
}

$Failures = @()

$RequiredPresentationTokens = @(
    @{ Label = "Persistent engine component"; Token = "EngineAudioComponent" },
    @{ Label = "Persistent procedural wave"; Token = "EngineProceduralWave" },
    @{ Label = "Attached engine voice"; Token = "SpawnSoundAttached" },
    @{ Label = "Buffered procedural engine"; Token = "QueueEngineAudioIfNeeded" },
    @{ Label = "Engine priority override"; Token = "bOverridePriority = true" },
    @{ Label = "Engine survives voice pressure"; Token = "bShouldRemainActiveIfDropped = true" },
    @{ Label = "Persistent engine runtime hook"; Token = "RI AUDIO ENGINE channel=persistent_procedural" }
)

foreach ($Check in $RequiredPresentationTokens) {
    $Hit = Select-String -Path @($PresentationCpp, $PresentationHeader) -SimpleMatch $Check.Token
    if ($Hit) {
        Write-Host ("[PASS] {0}" -f $Check.Label) -ForegroundColor Green
    }
    else {
        Write-Host ("[FAIL] {0} - token missing: {1}" -f $Check.Label, $Check.Token) -ForegroundColor Red
        $Failures += $Check.Label
    }
}

$LegacyEnginePulseOwner = Select-String -Path $PresentationCpp -SimpleMatch 'TEXT("EnginePulse")'
if ($LegacyEnginePulseOwner) {
    Write-Host "[FAIL] Presentation still fires one-shot EnginePulse events." -ForegroundColor Red
    $LegacyEnginePulseOwner | ForEach-Object { Write-Host ("       " + $_.Line.Trim()) -ForegroundColor DarkGray }
    $Failures += "legacy engine pulse owner"
}
else {
    Write-Host "[PASS] No one-shot EnginePulse owner in presentation" -ForegroundColor Green
}

$MovementAudioOwnership = Select-String -Path $MovementCpp -Pattern 'RIAudioEvents|EnginePulse|TireSkid' -CaseSensitive:$false
if ($MovementAudioOwnership) {
    Write-Host "[FAIL] Bike movement contains presentation-audio ownership." -ForegroundColor Red
    $MovementAudioOwnership | Select-Object -First 8 | ForEach-Object {
        Write-Host ("       " + $_.Line.Trim()) -ForegroundColor DarkGray
    }
    $Failures += "movement audio ownership"
}
else {
    Write-Host "[PASS] Bike movement remains physics-only" -ForegroundColor Green
}

$TransientOwner = Select-String -Path $AudioEventsCpp -SimpleMatch 'namespace RIAudioEvents'
if ($TransientOwner) {
    Write-Host "[PASS] Transient audio owner present" -ForegroundColor Green
}
else {
    Write-Host "[FAIL] RIAudioEvents transient owner missing" -ForegroundColor Red
    $Failures += "transient owner"
}

if ($Failures.Count -gt 0) {
    Write-Host ""
    Write-Host "Audio contract verification FAILED." -ForegroundColor Red
    $Failures | Select-Object -Unique | ForEach-Object { Write-Host " - $_" -ForegroundColor Red }
    exit 2
}

Write-Host ""
Write-Host "Audio contract verification PASSED." -ForegroundColor Green
Write-Host "Engine: persistent presentation channel" -ForegroundColor DarkGray
Write-Host "Transient effects: RIAudioEvents" -ForegroundColor DarkGray
Write-Host "Bike movement: physics-only" -ForegroundColor DarkGray
exit 0
