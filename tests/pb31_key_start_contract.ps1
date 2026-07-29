$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$header = Get-Content -Raw (Join-Path $root 'core\inc\blue.h')
$blue = Get-Content -Raw (Join-Path $root 'core\src\blue.c')
$main = Get-Content -Raw (Join-Path $root 'main.c')

if ($header -notmatch 'void\s+Key_Start\s*\(void\)\s*;') {
    throw 'blue.h must declare Key_Start.'
}

if ($blue -notmatch '(?s)void\s+Key_Start\s*\(void\)\s*\{\s*BT_Start\s*\(\s*\)\s*;\s*\}') {
    throw 'Key_Start must reuse BT_Start.'
}

if ($main -notmatch '(?s)void\s+GROUP1_IRQHandler\s*\(void\)\s*\{.*?key_user_key_PIN.*?DL_GPIO_clearInterruptStatus\s*\(\s*key_user_key_PORT\s*,\s*key_user_key_PIN\s*\)\s*;.*?Key_Start\s*\(\s*\)\s*;') {
    throw 'GROUP1_IRQHandler must clear PB31 and invoke Key_Start.'
}

Write-Output 'PB31 key-start contract passed.'
