$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$followLine = Get-Content -Raw (Join-Path $root 'core\src\follow_line.c')
$main = Get-Content -Raw (Join-Path $root 'main.c')
$motor = Get-Content -Raw (Join-Path $root 'core\src\MG513XGMR.c')

if ($followLine -notmatch 'LineController_t\s+g_question2_line_controller\s*;') {
    throw 'Question 2 controller must use zero initialization.'
}

if ($followLine -notmatch 'LineController_t\s+g_question3_line_controller\s*;') {
    throw 'Question 3 controller must use zero initialization.'
}

if ($followLine -match '(?s)LineController_t\s+g_question[23]_line_controller\s*=\s*\{') {
    throw 'Follow-line PID parameters must not have global initializer defaults.'
}

if ($main -match 'PID_init\s*\(\s*&g_motor_(left|right)\.speed_pid') {
    throw 'main.c must not initialize motor speed PID before MG513XGMR_Init.'
}

if ($motor -notmatch '(?s)void\s+MG513XGMR_Init\s*\([^\)]*\).*?PID_init\s*\(\s*&motor->speed_pid') {
    throw 'MG513XGMR_Init must remain the motor speed PID initialization source.'
}

Write-Output 'PID parameter source contract passed.'
