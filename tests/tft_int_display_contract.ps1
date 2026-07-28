$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$driver = Get-Content -Raw (Join-Path $root 'core\src\zf_device_tft180.c')
$header = Get-Content -Raw (Join-Path $root 'core\inc\zf_device_tft180.h')
$main = Get-Content -Raw (Join-Path $root 'main.c')
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

if ($header -notmatch 'void\s+tft_print\s*\(\s*uint16\s+x\s*,\s*uint16\s+y\s*,\s*tft180_font_size_enum\s+size\s*,\s*const\s+char\s*\*format\s*,\s*\.\.\.\s*\)') {
    throw 'TFT header must expose printf-style tft_print.'
}

if ($driver -notmatch '(?s)void\s+tft_print\s*\([^\)]*\.\.\.\s*\).*?vsnprintf\s*\(') {
    throw 'tft_print must format its output with vsnprintf.'
}

if ($driver -notmatch 'tft180_show_fixed_string\s*\(\s*x\s*,\s*y\s*,\s*buffer\s*,\s*display_length\s*\)') {
    throw 'tft_print must redraw to the previous text length to clear stale characters.'
}

if ($main -match 'tft180_show_(string|int|uint|float)\s*\(') {
    throw 'main.c display calls must use tft_print.'
}

if ($main -notmatch 'tft_print\s*\(') {
    throw 'main.c must render through tft_print.'
}

if ($driver -notmatch '#define\s+TFT180_PRINT_SLOT_COUNT\s+32U') {
    throw 'tft_print must retain enough display positions for the current screen.'
}

Write-Output 'TFT signed integer refresh contract passed.'
