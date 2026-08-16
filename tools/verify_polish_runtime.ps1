param(
    [string]$ProjectRoot = "C:\GameDev\Roadside-Idiots"
)

$ErrorActionPreference = "Stop"

$LogDirectory = Join-Path $ProjectRoot "Saved\Logs"
if (-not (Test-Path $LogDirectory)) {
    Write-Host "[FAIL] Log directory not found: $LogDirectory" -ForegroundColor Red
    exit 1
}

$Log = Get-ChildItem (Join-Path $LogDirectory "*.log") -File |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if (-not $Log) {
    Write-Host "[FAIL] No Unreal log found in $LogDirectory" -ForegroundColor Red
    exit 1
}

Write-Host "Roadside Idiots - Polish Runtime Verification" -ForegroundColor Cyan
Write-Host "Log: $($Log.FullName)"
Write-Host "Modified: $($Log.LastWriteTime)"
Write-Host ""

$Checks = @(
    @{ Label = "Controller/menu input flow"; Pattern = "RI INPUT FLOW"; Required = $true },
    @{ Label = "Landmark world layer"; Pattern = "RI WORLD LANDMARKS"; Required = $true },
    @{ Label = "World landmark signage"; Pattern = "RI WORLD SIGNAGE"; Required = $true },
    @{ Label = "Near roadside facade details"; Pattern = "RI WORLD FACADE_DETAILS"; Required = $true },
    @{ Label = "Approved free vegetation loaded"; Pattern = "source=approved_free"; Required = $true },
    @{ Label = "Distant roadside backdrop"; Pattern = "RI WORLD BACKDROP"; Required = $true },
    @{ Label = "Instanced road markings"; Pattern = "RI ROAD MARKINGS"; Required = $true },
    @{ Label = "Road surface detail"; Pattern = "RI ROAD SURFACE_DETAIL"; Required = $true },
    @{ Label = "Presentation color grade"; Pattern = "RI PRESENTATION GRADE"; Required = $true },
    @{ Label = "Rival personality identity"; Pattern = "RI RIVAL IDENTITY"; Required = $true },
    @{ Label = "Traffic visual polish"; Pattern = "RI TRAFFIC VISUAL"; Required = $false },
    @{ Label = "Tapered traffic shell"; Pattern = "style=tapered_shell"; Required = $false },
    @{ Label = "Item balance"; Pattern = "RI ITEMS BALANCE"; Required = $true },
    @{ Label = "Playtest start"; Pattern = "RI PLAYTEST START"; Required = $true },
    @{ Label = "Playtest summary"; Pattern = "RI PLAYTEST SUMMARY"; Required = $false },
    @{ Label = "Damage-source summary"; Pattern = "RI PLAYTEST DAMAGE_SOURCES"; Required = $false },
    @{ Label = "Competition summary"; Pattern = "RI PLAYTEST COMPETITION"; Required = $false },
    @{ Label = "Advance traffic warning"; Pattern = "RI TRAFFIC WARN"; Required = $false },
    @{ Label = "Finish celebration"; Pattern = "RI FINISH CELEBRATION"; Required = $false },
    @{ Label = "Finish restart input"; Pattern = "RI INPUT FINISH_RESTART"; Required = $false }
)

$MissingRequired = @()
foreach ($Check in $Checks) {
    $Matches = Select-String -Path $Log.FullName -SimpleMatch $Check.Pattern
    if ($Matches) {
        Write-Host ("[PASS] {0} ({1})" -f $Check.Label, $Matches.Count) -ForegroundColor Green
        $Matches | Select-Object -Last 3 | ForEach-Object {
            Write-Host ("       " + $_.Line.Trim()) -ForegroundColor DarkGray
        }
    }
    else {
        $Status = if ($Check.Required) { "FAIL" } else { "INFO" }
        $Color = if ($Check.Required) { "Red" } else { "Yellow" }
        Write-Host ("[{0}] {1} - no matching line" -f $Status, $Check.Label) -ForegroundColor $Color
        if ($Check.Required) {
            $MissingRequired += $Check.Label
        }
    }
}

Write-Host ""
Write-Host "Warning regression checks" -ForegroundColor Cyan

$ForbiddenWarningPatterns = @(
    "has to have 'CollisionEnabled' set to 'Query and Physics'",
    "Trying to simulate physics on",
    "Simulate Physics but Collision Enabled is incompatible",
    "RI FREE VEGETATION skipped"
)

$WarningFailures = @()
foreach ($Pattern in $ForbiddenWarningPatterns) {
    $Hits = Select-String -Path $Log.FullName -SimpleMatch $Pattern
    if ($Hits) {
        Write-Host ("[FAIL] Runtime regression: {0} hit(s) for '{1}'" -f $Hits.Count, $Pattern) -ForegroundColor Red
        $Hits | Select-Object -Last 5 | ForEach-Object {
            Write-Host ("       " + $_.Line.Trim()) -ForegroundColor DarkGray
        }
        $WarningFailures += $Pattern
    }
}

if ($WarningFailures.Count -eq 0) {
    Write-Host "[PASS] No known presentation physics/collision/asset-loading regression" -ForegroundColor Green
}

Write-Host ""
if ($MissingRequired.Count -gt 0 -or $WarningFailures.Count -gt 0) {
    if ($MissingRequired.Count -gt 0) {
        Write-Host "Runtime verification failed required checks:" -ForegroundColor Red
        $MissingRequired | ForEach-Object { Write-Host " - $_" -ForegroundColor Red }
    }
    if ($WarningFailures.Count -gt 0) {
        Write-Host "Runtime verification found a known presentation/input/asset regression." -ForegroundColor Red
    }
    exit 2
}

Write-Host "Required runtime presentation/input hooks were observed." -ForegroundColor Green
Write-Host "Finish/traffic/restart entries are event-dependent and may legitimately be absent if they did not occur." -ForegroundColor DarkGray
exit 0
