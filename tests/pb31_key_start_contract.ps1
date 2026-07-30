$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$keyHeader = Get-Content -Raw (Join-Path $root 'core\inc\key.h')
$key = Get-Content -Raw (Join-Path $root 'core\src\key.c')
$main = Get-Content -Raw (Join-Path $root 'main.c')

if ($keyHeader -notmatch 'KEY_EVENT_USER') {
    throw 'key.h must define the user-key event bit.'
}

if ($key -notmatch '(?s)key_user_key_PIN.*?DL_GPIO_clearInterruptStatus\s*\(\s*GPIOB\s*,\s*key_user_key_PIN\s*\).*?g_key_event\s*\|=\s*KEY_EVENT_USER') {
    throw 'key.c must record the PB31 user-key event.'
}

if ($main -match 'Key_Start\s*\(\s*\)') {
    throw 'PB31 must not start the car while key functions are deferred.'
}

Write-Output 'PB31 key-event contract passed.'
