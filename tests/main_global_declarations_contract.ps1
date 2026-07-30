$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$mainHeader = Get-Content -Raw (Join-Path $root 'core\inc\main.h')
$blueHeader = Get-Content -Raw (Join-Path $root 'core\inc\blue.h')
$followLineHeader = Get-Content -Raw (Join-Path $root 'core\inc\follow_line.h')
$motorHeader = Get-Content -Raw (Join-Path $root 'core\inc\MG513XGMR.h')
$main = Get-Content -Raw (Join-Path $root 'main.c')
$blue = Get-Content -Raw (Join-Path $root 'core\src\blue.c')
$followLine = Get-Content -Raw (Join-Path $root 'core\src\follow_line.c')
$motor = Get-Content -Raw (Join-Path $root 'core\src\MG513XGMR.c')

$requiredDeclarations = @(
    'extern\s+Grayscale_Sensor_t\s+g_grayscale_sensor\s*;',
    'extern\s+volatile\s+uint32_t\s+g_system_timer_sec\s*;',
    'extern\s+volatile\s+uint8_t\s+g_bluetooth_data\s*;',
    'extern\s+volatile\s+uint8_t\s+g_bt_color_mode\s*;',
    'extern\s+volatile\s+uint8_t\s+g_bt_speed_grade\s*;',
    'extern\s+volatile\s+uint8_t\s+g_bt_running_flag\s*;',
    'extern\s+LineController_t\s+g_question2_line_controller\s*;',
    'extern\s+LineController_t\s+g_question3_line_controller\s*;',
    'extern\s+LineController_t\s*\*\s*g_active_line_controller\s*;',
    'extern\s+float\s+g_turn_output\s*;',
    'extern\s+MG513XGMR_Motor_t\s+g_motor_left\s*;',
    'extern\s+MG513XGMR_Motor_t\s+g_motor_right\s*;'
)

foreach ($declaration in $requiredDeclarations) {
    if ($mainHeader -notmatch $declaration) {
        throw "main.h is missing global declaration: $declaration"
    }
}

foreach ($header in @($blueHeader, $followLineHeader, $motorHeader)) {
    if ($header -match '(?m)^\s*extern\s+.*\bg_[A-Za-z0-9_]+\s*;' ) {
        throw 'Module headers must not duplicate global declarations.'
    }
}

if ($main -notmatch 'Grayscale_Sensor_t\s+g_grayscale_sensor\s*;') { throw 'main.c must define g_grayscale_sensor.' }
if ($blue -notmatch 'volatile\s+uint8_t\s+g_bluetooth_data\s*=') { throw 'blue.c must define g_bluetooth_data.' }
if ($followLine -notmatch 'LineController_t\s+g_question2_line_controller\s*;') { throw 'follow_line.c must define question 2.' }
if ($motor -notmatch 'MG513XGMR_Motor_t\s+g_motor_left\s*;') { throw 'MG513XGMR.c must define left motor.' }

Write-Output 'Main global declarations contract passed.'
