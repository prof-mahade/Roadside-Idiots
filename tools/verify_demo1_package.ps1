param(
    [string]$PackagePath = "",
    [string]$ArchiveRoot = "C:\GameDev\RoadsideIdiots_Packaged",
    [switch]$Launch
)

$ErrorActionPreference = "Stop"

function Fail([string]$Message) {
    throw "DEMO 1 VERIFY FAILED: $Message"
}

function RequireLiteral([string]$Text, [string]$Phrase, [string]$FailureMessage) {
    if ([string]::IsNullOrEmpty($Text) -or -not $Text.Contains($Phrase)) {
        Fail $FailureMessage
    }
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

$ForbiddenLooseFiles = @(Get-ChildItem -Path $PackagePath -Recurse -Force -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -match '(?i)Sankool|CompoundWall_Kit' })
if ($ForbiddenLooseFiles.Count -gt 0) {
    $ForbiddenLooseFiles | Select-Object -First 10 | ForEach-Object {
        Write-Host "       $($_.FullName)" -ForegroundColor Red
    }
    Fail "forbidden content name was found in the packaged directory."
}

$UnexpectedSourceFiles = @(Get-ChildItem -Path $PackagePath -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -in @(".cpp", ".h") })
if ($UnexpectedSourceFiles.Count -gt 0) {
    $UnexpectedSourceFiles | Select-Object -First 10 | ForEach-Object {
        Write-Host "       $($_.FullName)" -ForegroundColor Red
    }
    Fail "C++ source/header files were unexpectedly included in the distributable package."
}

$Prereq = Get-ChildItem -Path $PackagePath -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match "UEPrereq.*\.exe" } |
    Select-Object -First 1

# Tester ZIPs must not contain private debug symbols. They are useful locally but
# unnecessarily inflate the public artifact and expose implementation symbols.
$PdbFiles = @(Get-ChildItem -Path $PackagePath -Recurse -File -Filter "*.pdb" -ErrorAction SilentlyContinue)
if ($PdbFiles.Count -gt 0) {
    $PdbFiles | Select-Object -First 10 | ForEach-Object {
        Write-Host "       $($_.FullName)" -ForegroundColor Red
    }
    Fail "debug PDB file(s) remain in the player-test package. Run finalize_player_test_package.ps1 before sharing."
}

$Manifest = Join-Path $PackagePath "DEMO1_BUILD_INFO.txt"
$Readme = Join-Path $PackagePath "README_ROADSIDE_IDIOTS.txt"
$TestPlan = Join-Path $PackagePath "PLAYER_TEST_PLAN.md"
$FeedbackForm = Join-Path $PackagePath "PLAYER_TEST_FEEDBACK_FORM.md"
$PlayerTestEvidence = Join-Path $PackagePath "PLAYER_TEST_BUILD_INFO.txt"
$BugfixEvidence = Join-Path $PackagePath "BUGFIX_PREFLIGHT.txt"
$ZipPath = "$PackagePath.zip"
$ChecksumPath = "$ZipPath.sha256.txt"

foreach ($RequiredFile in @($Manifest, $Readme, $TestPlan, $FeedbackForm, $PlayerTestEvidence, $BugfixEvidence, $ZipPath, $ChecksumPath)) {
    if (-not (Test-Path $RequiredFile)) {
        Fail "required player-test artifact is missing: $RequiredFile"
    }
}

$ManifestText = Get-Content $Manifest -Raw
if ($ManifestText -notmatch 'Input contract preflight:\s*PASSED') {
    Fail "Build manifest does not record a passed input-contract preflight."
}
if ($ManifestText -match '(?i)Sankool|CompoundWall_Kit') {
    $NonPolicyForbidden = Get-Content $Manifest | Where-Object {
        $_ -match '(?i)Sankool|CompoundWall_Kit' -and $_ -notmatch '(?i)forbidden|blocks'
    }
    if ($NonPolicyForbidden) {
        Fail "Build manifest contains an unexpected forbidden-content reference."
    }
}

$EvidenceText = Get-Content $PlayerTestEvidence -Raw
$RequiredEvidencePhrases = @(
    'Tracked working tree clean before packaging: YES',
    'Approved free vegetation assets present before packaging: 4/4',
    'Combined input/audio/lifecycle preflight: PASSED',
    'Packaging pipeline: tools/package_player_test.ps1',
    'Tester feedback form included: YES'
)
foreach ($Phrase in $RequiredEvidencePhrases) {
    RequireLiteral $EvidenceText $Phrase "PLAYER_TEST_BUILD_INFO.txt is missing reproducibility evidence: $Phrase"
}

$BugfixText = Get-Content $BugfixEvidence -Raw
$RequiredBugfixPhrases = @(
    'Input contract verification PASSED.',
    'Audio contract verification PASSED.',
    '[PASS] BUGFIX CONTRACT PREFLIGHT'
)
foreach ($Phrase in $RequiredBugfixPhrases) {
    RequireLiteral $BugfixText $Phrase "BUGFIX_PREFLIGHT.txt is missing required result: $Phrase"
}

$ReadmeText = Get-Content $Readme -Raw
$RequiredReadmePhrases = @(
    'Y              quick race again after finish',
    'B              throw rotten egg / menu back / resume; Main Menu after finish',
    'Esc            pause / menu back; Main Menu after finish',
    'Enter / A / Y  race again with the same configured setup',
    'Esc / B        return to Main Menu without auto-starting another race'
)
foreach ($Phrase in $RequiredReadmePhrases) {
    RequireLiteral $ReadmeText $Phrase "Packaged README is missing current controller/Main Menu guidance: $Phrase"
}

$FeedbackText = Get-Content $FeedbackForm -Raw
foreach ($Phrase in @(
    'Engine remains audible while horn/item/crash sounds play',
    'Pause menu contains an explicit **MAIN MENU** option',
    'After finish, Esc/B returns to Main Menu',
    'Y after finish restarts the same configured race',
    'Would you voluntarily play another race right now?'
)) {
    RequireLiteral $FeedbackText $Phrase "Packaged feedback form is missing a required test question: $Phrase"
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

Add-Type -AssemblyName System.IO.Compression.FileSystem
$Zip = [System.IO.Compression.ZipFile]::OpenRead($ZipPath)
try {
    $ZipEntries = @($Zip.Entries | ForEach-Object { $_.FullName.Replace('/', '\') })

    $RequiredZipEntries = @(
        @{ Label = 'RoadsideIdiots.exe'; Pattern = '(^|\\)RoadsideIdiots\.exe$' },
        @{ Label = 'DEMO1_BUILD_INFO.txt'; Pattern = '(^|\\)DEMO1_BUILD_INFO\.txt$' },
        @{ Label = 'README_ROADSIDE_IDIOTS.txt'; Pattern = '(^|\\)README_ROADSIDE_IDIOTS\.txt$' },
        @{ Label = 'PLAYER_TEST_PLAN.md'; Pattern = '(^|\\)PLAYER_TEST_PLAN\.md$' },
        @{ Label = 'PLAYER_TEST_FEEDBACK_FORM.md'; Pattern = '(^|\\)PLAYER_TEST_FEEDBACK_FORM\.md$' },
        @{ Label = 'PLAYER_TEST_BUILD_INFO.txt'; Pattern = '(^|\\)PLAYER_TEST_BUILD_INFO\.txt$' },
        @{ Label = 'BUGFIX_PREFLIGHT.txt'; Pattern = '(^|\\)BUGFIX_PREFLIGHT\.txt$' }
    )

    foreach ($Required in $RequiredZipEntries) {
        $Pattern = $Required.Pattern
        $Found = @($ZipEntries | Where-Object { $_ -match $Pattern }).Count -gt 0
        if (-not $Found) {
            Fail "shareable ZIP is missing required entry: $($Required.Label)"
        }
    }

    $ForbiddenZipEntries = @($ZipEntries | Where-Object { $_ -match '(?i)Sankool|CompoundWall_Kit' })
    if ($ForbiddenZipEntries.Count -gt 0) {
        $ForbiddenZipEntries | Select-Object -First 10 | ForEach-Object {
            Write-Host "       $_" -ForegroundColor Red
        }
        Fail "forbidden content name was found inside the shareable ZIP."
    }

    $PdbZipEntries = @($ZipEntries | Where-Object { $_ -match '(?i)(^|\\)[^\\]+\.pdb$' })
    if ($PdbZipEntries.Count -gt 0) {
        $PdbZipEntries | Select-Object -First 10 | ForEach-Object { Write-Host "       $_" -ForegroundColor Red }
        Fail "debug PDB file(s) remain inside the shareable ZIP."
    }
}
finally {
    $Zip.Dispose()
}

Write-Host ""
Write-Host "ROADSIDE IDIOTS - PLAYER TEST PACKAGE VERIFY" -ForegroundColor Cyan
Write-Host "Package       : $PackagePath"
Write-Host "Executable    : $($Exe.FullName)" -ForegroundColor Green
Write-Host "Cooked files  : $($Containers.Count)" -ForegroundColor Green
Write-Host "Prerequisite  : $(if ($Prereq) { $Prereq.FullName } else { 'not found / may be bundled differently' })"
Write-Host "Manifest      : $Manifest" -ForegroundColor Green
Write-Host "Player README : $Readme" -ForegroundColor Green
Write-Host "Test plan     : $TestPlan" -ForegroundColor Green
Write-Host "Feedback form : $FeedbackForm" -ForegroundColor Green
Write-Host "Build evidence: $PlayerTestEvidence" -ForegroundColor Green
Write-Host "Bugfix proof  : $BugfixEvidence" -ForegroundColor Green
Write-Host "Share ZIP     : $ZipPath ($([math]::Round($ZipInfo.Length / 1MB, 1)) MB)" -ForegroundColor Green
Write-Host "SHA-256       : $ActualHash" -ForegroundColor Green
Write-Host "ZIP contents  : required artifacts present; no PDB/source/forbidden names" -ForegroundColor Green
Write-Host ""

Write-Host "----- PLAYER TEST BUILD INFO -----" -ForegroundColor DarkCyan
Get-Content $PlayerTestEvidence | ForEach-Object { Write-Host $_ }
Write-Host "----------------------------------" -ForegroundColor DarkCyan
Write-Host ""

Write-Host "STATIC PACKAGE + ZIP CHECK: PASSED" -ForegroundColor Green
Write-Host ""
Write-Host "STANDALONE HUMAN SMOKE GATE" -ForegroundColor Yellow
Write-Host "  1. Launch from the packaged executable, not Unreal Editor."
Write-Host "  2. Setup/menu works with keyboard and Xbox-style controller."
Write-Host "  3. During an unfinished race, Y must do nothing."
Write-Host "  4. Engine must remain continuously audible underneath horn/item/crash sounds."
Write-Host "  5. Gameplay: A=peel, B=egg, X=recover, LB/RB=slap, P/Start=pause."
Write-Host "  6. Pause menu must show MAIN MENU; selecting it returns to setup without auto-start."
Write-Host "  7. Finish a race. P/Start must NOT replace the finish result with Pause."
Write-Host "  8. After finish, peel/egg/slap/recovery inputs must be blocked."
Write-Host "  9. After finish, Esc/B must return to Main Menu; Y/A/Enter must race again."
Write-Host " 10. Drive over repair patches/skid marks: absolutely zero physical bump."
Write-Host " 11. Run a busy race: no recurring AI wall oscillation or traffic-induced ping-pong."
Write-Host " 12. Confirm PN vegetation, roadside details, traffic shell and road markings render."
Write-Host " 13. Confirm no forbidden/paid pack identity appears anywhere in the build."
Write-Host " 14. Give the tester PLAYER_TEST_FEEDBACK_FORM.md after the session."
Write-Host ""

if ($Launch) {
    Write-Host "Launching packaged player-test build..." -ForegroundColor Cyan
    Start-Process -FilePath $Exe.FullName -WorkingDirectory $Exe.DirectoryName -ArgumentList @("-windowed", "-ResX=1280", "-ResY=720")
}
else {
    Write-Host "To launch automatically:" -ForegroundColor DarkGray
    Write-Host "  .\tools\verify_demo1_package.ps1 -PackagePath `"$PackagePath`" -Launch" -ForegroundColor DarkGray
}
