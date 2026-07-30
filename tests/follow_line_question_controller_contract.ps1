$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$header = Get-Content -Raw (Join-Path $root 'core\inc\follow_line.h')
$source = Get-Content -Raw (Join-Path $root 'core\src\follow_line.c')
$blue = Get-Content -Raw (Join-Path $root 'core\src\blue.c')
$main = Get-Content -Raw (Join-Path $root 'main.c')

if ($header -notmatch 'int\s+base_speed\s*;') {
    throw 'LineController_t must own base_speed.'
}

if ($source -notmatch 'LineController_t\s+g_question2_line_controller\s*;') {
    throw 'Question 2 controller is missing.'
}

if ($source -notmatch 'LineController_t\s+g_question3_line_controller\s*;') {
    throw 'Question 3 controller is missing.'
}

if ($main -notmatch 'FollowLine_Init\s*\(\s*&g_question3_line_controller') {
    throw 'Question 3 must have an independent controller initialization.'
}

if ($main -notmatch 'FollowLine_Select_Question\s*\(\s*3U\s*\)') {
    throw 'Question 3 must be selected after initialization.'
}

if ($blue -notmatch "(?s)case\s+'2'\s*:.*?FollowLine_Select_Question\s*\(\s*2U\s*\)") {
    throw 'Bluetooth command 2 must select question 2.'
}

if ($blue -notmatch "(?s)case\s+'3'\s*:.*?FollowLine_Select_Question\s*\(\s*3U\s*\)") {
    throw 'Bluetooth command 3 must select question 3.'
}

if ($blue -notmatch 'g_active_line_controller->kp') {
    throw 'Bluetooth PID adjustment must target the active controller.'
}

if ($main -match 'FollowLine_Update\s*\([^\)]*,\s*880\s*\)') {
    throw 'ChassisTask must not hard-code base speed 880.'
}

Write-Output 'Question line-controller contract passed.'
