# Builds everything: the secs library, the unit tests and the benchmarks.
#
# Usage: scripts/build-all.ps1 [-BuildType Debug|Release|...] [-BuildDir build]

param(
    [string]$BuildType = "Debug",
    [string]$BuildDir = "build"
)

# Note: no $ErrorActionPreference = "Stop" — in Windows PowerShell 5.1 it turns
# any stderr output from native tools (conan logs to stderr) into a fatal error.
# Failures are detected through $LASTEXITCODE instead.

$Root = Split-Path -Parent $PSScriptRoot
$BuildPath = Join-Path $Root $BuildDir

Write-Host "==> Installing dependencies via Conan ($BuildType)..."
conan install $Root --output-folder=$BuildPath --build=missing -s build_type=$BuildType
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Configuring secs ($BuildType) with tests and benchmarks..."
cmake -S $Root -B $BuildPath `
    -DCMAKE_BUILD_TYPE=$BuildType `
    -DCMAKE_TOOLCHAIN_FILE="$BuildPath/conan_toolchain.cmake" `
    -DCMAKE_PREFIX_PATH="$BuildPath" `
    -DBUILD_GTESTS=ON `
    -DBUILD_BENCHMARKS=ON
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Building all targets..."
cmake --build $BuildPath --config $BuildType
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Build complete: $BuildPath"
