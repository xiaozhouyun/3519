$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$mainHeader = Get-Content -Raw (Join-Path $root 'core\inc\main.h')
$motor = Get-Content -Raw (Join-Path $root 'core\src\MG513XGMR.c')

if ($mainHeader -notmatch 'extern\s+float\s+g_odometer_mm\s*;') {
    throw 'main.h must declare g_odometer_mm.'
}

if ($motor -notmatch 'float\s+g_odometer_mm\s*=\s*0\.0f\s*;') {
    throw 'MG513XGMR.c must define g_odometer_mm.'
}

if ($motor -notmatch 'g_odometer_mm\s*=\s*0\.0f\s*;') {
    throw 'MG513XGMR_Init must clear g_odometer_mm.'
}

if ($motor -notmatch '69\.0f') {
    throw 'Odometer must use the 69 mm wheel radius.'
}

if ($motor -notmatch 'MG513XGMR_PULSES_PER_REV') {
    throw 'Odometer must use the encoder pulses-per-revolution constant.'
}

if ($motor -notmatch '(?s)wheel_delta\[MG513XGMR_LEFT\].*wheel_delta\[MG513XGMR_RIGHT\]') {
    throw 'Odometer must average the signed left and right wheel deltas.'
}

Write-Output 'Odometer contract passed.'
