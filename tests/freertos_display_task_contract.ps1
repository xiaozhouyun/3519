$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$main = Get-Content -Raw (Join-Path $root 'main.c')
$project = Get-Content -Raw (Join-Path $root 'keil\empty_LP_MSPM0G3519_nortos_keil.uvprojx')

function Require-Match([string]$text, [string]$pattern, [string]$message) {
    if ($text -notmatch $pattern) {
        throw $message
    }
}

Require-Match $main '#include\s+"FreeRTOS\.h"' 'main.c must include FreeRTOS.h.'
Require-Match $main '#include\s+"task\.h"' 'main.c must include task.h.'
Require-Match $main 'static\s+void\s+DisplayTask\s*\(\s*void\s*\*' 'main.c must define DisplayTask.'
Require-Match $main 'xTaskCreate\s*\(\s*DisplayTask' 'main.c must create DisplayTask.'
Require-Match $main 'vTaskStartScheduler\s*\(' 'main.c must start the FreeRTOS scheduler.'
Require-Match $main 'vTaskDelay\s*\(\s*pdMS_TO_TICKS\s*\(\s*100' 'DisplayTask must delay for 100 ms.'
if ($main -match 'delay_cycles\s*\(\s*CPUCLK_FREQ\s*/\s*10\s*\)') {
    throw 'main.c must not retain the busy-wait display delay.'
}

Require-Match $project '<TargetName>empty_LP_MSPM0G3519_freertos_keil</TargetName>' 'Keil project must identify the FreeRTOS target.'
Require-Match $project 'freertos_builds_LP_MSPM0G3519_release_keil\.lib' 'FreeRTOS target must link the matching SDK library.'

Write-Output 'FreeRTOS DisplayTask contract passed.'
