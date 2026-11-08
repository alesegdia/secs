# Builds the test binary and runs only the audit regression tests
# (src/gtests/AuditTests.cpp, suites named Audit*).
#
# Usage: scripts/run-audit-tests.ps1 [-BuildType Debug] [-BuildDir build] [extra gtest args...]
#   e.g. scripts/run-audit-tests.ps1 -BuildType Debug --gtest_filter=Audit04*
#
# Use a Debug build: some audit tests rely on Debug bounds checks to detect
# out-of-bounds reads and may pass by accident in Release.
#
# Requires dependencies to be installed already (run scripts/build-all.ps1 once).

# PositionalBinding=$false: -BuildType/-BuildDir must be named, so that bare
# gtest arguments (e.g. --gtest_filter=...) are never bound to them.
[CmdletBinding(PositionalBinding = $false)]
param(
    [string]$BuildType = "Debug",
    [string]$BuildDir = "build",
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$GTestArgs = @()
)

# Note: no $ErrorActionPreference = "Stop" — in Windows PowerShell 5.1 it turns
# any stderr output from native tools into a fatal error. Failures are
# detected through $LASTEXITCODE instead.

$Root = Split-Path -Parent $PSScriptRoot
$BuildPath = Join-Path $Root $BuildDir

if (-not (Test-Path (Join-Path $BuildPath "conan_toolchain.cmake"))) {
    Write-Error "$BuildPath/conan_toolchain.cmake not found. Run scripts/build-all.ps1 first."
    exit 1
}

Write-Host "==> Configuring secs ($BuildType) with tests..."
cmake -S $Root -B $BuildPath `
    -DCMAKE_BUILD_TYPE=$BuildType `
    -DCMAKE_TOOLCHAIN_FILE="$BuildPath/conan_toolchain.cmake" `
    -DCMAKE_PREFIX_PATH="$BuildPath" `
    -DBUILD_GTESTS=ON | Out-Null
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Building secs-tests..."
cmake --build $BuildPath --config $BuildType --target secs-tests
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# Multi-config generators (MSVC) put the binary in a config subdirectory;
# single-config generators (Make/Ninja) put it directly in the build folder.
$Candidates = @(
    (Join-Path $BuildPath "$BuildType/secs-tests.exe"),
    (Join-Path $BuildPath "$BuildType/secs-tests"),
    (Join-Path $BuildPath "secs-tests.exe"),
    (Join-Path $BuildPath "secs-tests")
)
$TestBin = $Candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $TestBin) {
    Write-Error "secs-tests binary not found in $BuildPath."
    exit 1
}

# Default to all audit suites; a --gtest_filter passed by the caller overrides it.
$RunArgs = @()
if (-not ($GTestArgs | Where-Object { $_ -like "--gtest_filter=*" })) {
    $RunArgs += "--gtest_filter=Audit*:*/Audit*"
}
$RunArgs += "--gtest_color=yes"
$RunArgs += $GTestArgs

Write-Host "==> Running audit tests: $TestBin"
& $TestBin @RunArgs
exit $LASTEXITCODE
