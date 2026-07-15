<#
============================================================================
QcPerf - Windows ARM64 build script
============================================================================
Wraps the exact command sequence documented in README.md > Compilation
Instructions > Windows ARM64 > Command Line, parameterized for reuse by the
PR sanity-build workflow and the tag-triggered release workflow.

Usage:
    qcperf/ci/build-windows-arm64.ps1 -Config <Debug|Release> [-BuildShared] [-Backends "THERMAL;POWER;DUMMY"]

Env:
    PROJECT_VERSION - optional, default "0.1.0.0" (matches CMake default)
============================================================================
#>
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("Debug", "Release")]
    [string]$Config,

    [switch]$BuildShared,

    [string]$Backends = "THERMAL;POWER;DUMMY"
)

$ErrorActionPreference = "Stop"

$ProjectVersion = $env:PROJECT_VERSION
if ([string]::IsNullOrEmpty($ProjectVersion)) {
    $ProjectVersion = "0.1.0.0"
}

$BuildSharedValue = "OFF"
$SharedSuffix = ""
if ($BuildShared) {
    $BuildSharedValue = "ON"
    $SharedSuffix = "-shared"
}

$BuildDir = "build-windows-arm64-$($Config.ToLower())$SharedSuffix"

Write-Host "==> Initializing submodules"
git submodule update --init --recursive

Write-Host "==> Configuring ($BuildDir)"
cmake -B $BuildDir -G "Visual Studio 17 2022" -A ARM64 `
    -DProjectVersion="$ProjectVersion" `
    -DBACKENDS="$Backends" `
    -DBUILD_SHARED="$BuildSharedValue"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Building ($BuildDir)"
cmake --build $BuildDir --config $Config
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "BUILD_DIR=$BuildDir"
