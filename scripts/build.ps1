param(
    [string]$BuildType = "Debug",
    [string]$BuildDir = "build"
)

$Root = Split-Path -Parent $PSScriptRoot
$BuildPath = Join-Path $Root $BuildDir

Write-Host "Installing dependencies via Conan..."
conan install $Root --output-folder=$BuildPath --build=missing -s build_type=$BuildType
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Configuring secs ($BuildType)..."
cmake -S $Root -B $BuildPath -DCMAKE_BUILD_TYPE=$BuildType -DCMAKE_TOOLCHAIN_FILE="$BuildPath/conan_toolchain.cmake" -DCMAKE_PREFIX_PATH="$BuildPath"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Building secs..."
cmake --build $BuildPath --config $BuildType
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Build complete: $BuildPath"
