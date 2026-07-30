$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$syscfg = Get-Content -Raw (Join-Path $root 'empty.syscfg')
$mainHeader = Get-Content -Raw (Join-Path $root 'core\inc\main.h')
$keyHeader = Get-Content -Raw (Join-Path $root 'core\inc\key.h')
$keySource = Get-Content -Raw (Join-Path $root 'core\src\key.c')
$main = Get-Content -Raw (Join-Path $root 'main.c')
$project = Get-Content -Raw (Join-Path $root 'keil\empty_LP_MSPM0G3519_nortos_keil.uvprojx')

if ($syscfg -notmatch '(?s)GPIO2\.associatedPins\[2\]\.interruptEn\s*=\s*true.*?GPIO2\.associatedPins\[2\]\.polarity\s*=\s*"FALL"') {
    throw 'PC6 key2 must use a falling-edge interrupt in SysConfig.'
}

if ($mainHeader -notmatch 'extern\s+volatile\s+uint8_t\s+g_key_event\s*;') {
    throw 'main.h must declare g_key_event.'
}

if ($keyHeader -notmatch 'void\s+Key_Init\s*\(void\)\s*;') {
    throw 'key.h must declare Key_Init.'
}

if ($keySource -notmatch 'volatile\s+uint8_t\s+g_key_event\s*=\s*0U\s*;') {
    throw 'key.c must define g_key_event.'
}

if ($keySource -notmatch '(?s)void\s+Key_Init\s*\(void\).*?NVIC_EnableIRQ\s*\(\s*key_GPIOB_INT_IRQN\s*\)') {
    throw 'Key_Init must enable the Group1 NVIC interrupt.'
}

foreach ($symbol in @('key_key0_PIN', 'key_key1_PIN', 'key_key2_PIN', 'key_key3_PIN', 'key_user_key_PIN')) {
    if ($keySource -notmatch $symbol) {
        throw "key.c must handle $symbol."
    }
}

if ($keySource -notmatch 'void\s+GROUP1_IRQHandler\s*\(void\)') {
    throw 'key.c must own GROUP1_IRQHandler.'
}

if ($main -match 'void\s+GROUP1_IRQHandler\s*\(void\)') {
    throw 'main.c must not define a second GROUP1_IRQHandler.'
}

if ($main -notmatch 'Key_Init\s*\(\s*\)\s*;') {
    throw 'main.c must initialize the key module.'
}

if ($project -notmatch '(?s)<FileName>key\.c</FileName>.*?<FilePath>\.\.\\core\\src\\key\.c</FilePath>') {
    throw 'Keil project must include key.c.'
}

Write-Output 'Key interrupt contract passed.'
