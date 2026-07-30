$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$main = Get-Content -Raw (Join-Path $root 'main.c')
$followHeader = Get-Content -Raw (Join-Path $root 'core\inc\follow_line.h')
$followSource = Get-Content -Raw (Join-Path $root 'core\src\follow_line.c')
$blue = Get-Content -Raw (Join-Path $root 'core\src\blue.c')

if ($main -notmatch 'QUESTION3_STOP_ODOMETER_MM\s+\(1640\.0f\)') {
    throw 'Question 3 stop distance must be 1640 mm.'
}

if ($main -notmatch 'QUESTION3_DECEL_DISTANCE_MM\s+\(200\.0f\)') {
    throw 'Question 3 must start decelerating 200 mm before the stop point.'
}

if ($main -notmatch 'QUESTION3_MIN_SPEED\s+\(200\)') {
    throw 'Question 3 must have a minimum deceleration speed.'
}

if ($main -match 'g_odometer_mm\s*>=\s*1638\.0f\s*&&') {
    throw 'A narrow odometer window can be skipped and must not control stopping.'
}

if ($main -notmatch '(?s)g_odometer_mm\s*>=\s*QUESTION3_STOP_ODOMETER_MM.*?BT_Stop\s*\(\s*\).*?MG513XGMR_Stop_All\s*\(\s*\)') {
    throw 'The target distance must stop the car immediately.'
}

if ($main -notmatch 'FollowLine_Update\s*\(\s*&g_grayscale_sensor\s*,\s*follow_speed\s*\)') {
    throw 'ChassisTask must pass the temporary decelerated speed to follow-line control.'
}

if ($followHeader -notmatch 'FollowLine_Update\s*\(\s*Grayscale_Sensor_t\s*\*sensor\s*,\s*int\s+base_speed\s*\)') {
    throw 'Follow-line API must accept a temporary base speed.'
}

if ($followSource -notmatch 'MG513XGMR_Set_Speed\s*\(\s*MG513XGMR_LEFT\s*,\s*base_speed\s*\+') {
    throw 'Follow-line source must use the temporary base speed.'
}

if ($blue -notmatch '(?s)void\s+BT_Start\s*\(void\)\s*\{.*?g_odometer_mm\s*=\s*0\.0f\s*;') {
    throw 'Each start must reset the odometer.'
}

Write-Output 'Question 3 odometer stop contract passed.'
