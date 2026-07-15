<#
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of Qualcomm Technologies, Inc. nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
cmake -S qcperf -B $BuildDir -G "Visual Studio 17 2022" -A ARM64 `
    -DProjectVersion="$ProjectVersion" `
    -DBACKENDS="$Backends" `
    -DBUILD_SHARED="$BuildSharedValue"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Building ($BuildDir)"
cmake --build $BuildDir --config $Config
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "BUILD_DIR=$BuildDir"
