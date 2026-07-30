$ErrorActionPreference = 'Stop'

$keyC = Get-Content -LiteralPath "$PSScriptRoot\..\core\src\key.c" -Raw
$keyH = Get-Content -LiteralPath "$PSScriptRoot\..\core\inc\key.h" -Raw
$mainH = Get-Content -LiteralPath "$PSScriptRoot\..\core\inc\main.h" -Raw

if ($keyH -notmatch '#define\s+KEY_DOUBLE_CLICK_MS\s+\(300U\)') {
    throw 'Missing 300 ms double-click window.'
}
if ($keyC -notmatch 'xTaskGetTickCountFromISR') {
    throw 'Key ISR does not use RTOS time for double-click detection.'
}
if ($keyC -notmatch 'Key_Is_Double_Click') {
    throw 'Missing double-click detection function.'
}
if ($keyC -match 'g_bt_running_flag\s*=\s*1U') {
    throw 'Key ISR directly starts the car instead of calling BT_Start after a double click.'
}
if ($keyC -notmatch 'FollowLine_Select_Question\(question\)\s*!=\s*false' -or
    $keyC -notmatch 'BT_Start\(\)') {
    throw 'The car must start only after question selection succeeds.'
}
if ($mainH -notmatch 'extern\s+volatile\s+uint8_t\s+g_key_double_click_pending\s*;') {
    throw 'Double-click pending state is not declared in main.h.'
}

Write-Output '[PASS] key double-click contract'
