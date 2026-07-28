$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$driver = Get-Content -Raw (Join-Path $root 'core\src\zf_device_tft180.c')
$showInt = [regex]::Match($driver, '(?s)void\s+tft180_show_int\s*\([^\)]*\)\s*\{.*?\n\}')

if (-not $showInt.Success) {
    throw 'TFT driver must define tft180_show_int.'
}

if ($showInt.Value -notmatch 'tft180_show_fixed_string\s*\(\s*x\s*,\s*y\s*,\s*data_buffer\s*,\s*num\s*\+\s*1\s*\)') {
    throw 'tft180_show_int must draw its full signed field, including trailing blanks.'
}

if ($driver -notmatch '(?s)void\s+tft180_show_uint\s*\([^\)]*\).*?tft180_show_fixed_string\s*\(\s*x\s*,\s*y\s*,\s*data_buffer\s*,\s*num\s*\)') {
    throw 'tft180_show_uint must overwrite its full unsigned field.'
}

if ($driver -notmatch '(?s)void\s+tft180_show_float\s*\([^\)]*\).*?tft180_show_fixed_string\s*\(\s*x\s*,\s*y\s*,\s*data_buffer\s*,\s*num\s*\+\s*pointnum\s*\+\s*2\s*\)') {
    throw 'tft180_show_float must overwrite its full floating-point field.'
}

Write-Output 'TFT signed integer refresh contract passed.'
