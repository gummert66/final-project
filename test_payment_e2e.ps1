Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Simple test harness helpers
$script:TestsTotal = 0
$script:TestsPassed = 0
$script:TestsFailed = 0

function Start-Test {
  param([string]$Name)
  Write-Host "Running: $Name"
}

function Assert-True {
  param([bool]$Condition, [string]$Name)
  $script:TestsTotal++
  if ($Condition) {
    $script:TestsPassed++
    Write-Host "[PASS] $Name"
  } else {
    $script:TestsFailed++
    Write-Host "[FAIL] $Name"
  }
}

# Backup existing CSV to isolate test run
if (Test-Path -LiteralPath 'paymentinfo.csv') {
  if (Test-Path -LiteralPath 'paymentinfo_backup.csv') { Remove-Item -Force 'paymentinfo_backup.csv' }
  Rename-Item -LiteralPath 'paymentinfo.csv' -NewName 'paymentinfo_backup.csv'
}

Start-Test 'Add payment through main menu'

# Prepare scripted input to drive the app
@'
1
Alice Smith
Internet
100
2024-01-31
0
'@ | Set-Content -NoNewline -LiteralPath 'e2e_input.txt'

# Run the binary with redirected input and capture output
if (-not (Test-Path -LiteralPath 'payment.exe')) {
  Write-Host 'payment.exe not found. Build the app before running e2e.'
  exit 1
}

cmd /c "payment.exe < e2e_input.txt" | Tee-Object -FilePath 'e2e_output.txt' | Out-Null

# Validate results
Assert-True (Test-Path -LiteralPath 'paymentinfo.csv') 'CSV file created after run'
Assert-True (Select-String -LiteralPath 'paymentinfo.csv' -Pattern 'Alice Smith' -Quiet) "CSV contains 'Alice Smith'"

# Optional: basic output contains some menu text (non-fatal if missing)
try {
  $containsMenu = Select-String -LiteralPath 'e2e_output.txt' -Pattern 'Add Payment' -Quiet
  Assert-True $containsMenu "Output shows 'Add Payment' option"
} catch {
  # If output wording changes, do not fail the suite hard
}

# Cleanup and restore
Remove-Item -Force 'e2e_input.txt','e2e_output.txt' -ErrorAction SilentlyContinue
if (Test-Path -LiteralPath 'paymentinfo_backup.csv') {
  if (Test-Path -LiteralPath 'paymentinfo.csv') { Remove-Item -Force 'paymentinfo.csv' }
  Rename-Item -LiteralPath 'paymentinfo_backup.csv' -NewName 'paymentinfo.csv'
}

# Summary
Write-Host ("E2E Summary: {0} passed, {1} failed, {2} total." -f $script:TestsPassed, $script:TestsFailed, $script:TestsTotal)
if ($script:TestsFailed -gt 0) { exit 1 } else { exit 0 }
