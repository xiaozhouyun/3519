$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$source = Get-Content -Raw (Join-Path $root 'core\src\drv8873.c')
$setSpeedPattern = '(?s)void\s+DRV8873_Set_Speed\s*\([^\)]*\)\s*\{.*?DRV8873_Control_t\s+ctrl\s*=\s*DRV8873_Speed_To_Control'

if ($source -notmatch $setSpeedPattern) {
    throw 'drv8873.c must define DRV8873_Set_Speed.'
}

if ($source -match '(?s)void\s+DRV8873_Set_Speed\s*\([^\)]*\)\s*\{.*?channel\s*==\s*DRV8873_CH1.*?speed\s*=\s*-speed.*?DRV8873_Control_t\s+ctrl\s*=\s*DRV8873_Speed_To_Control') {
    throw 'CH1 direction must not be inverted both before and during PH output mapping.'
}

if ($source -notmatch '(?s)channel\s*==\s*DRV8873_CH2.*?DRV8873_DIR_FORWARD.*?clearPins.*?PH2.*?else.*?setPins.*?PH2') {
    throw 'CH2 forward direction must drive PH2 low.'
}

if ($source -notmatch '(?s)else\s*\{\s*if\s*\(dir\s*==\s*DRV8873_DIR_FORWARD\)\s*\{\s*DL_GPIO_setPins.*?PH1.*?\}\s*else\s*\{\s*DL_GPIO_clearPins.*?PH1') {
    throw 'CH1 forward direction must drive PH1 high.'
}

Write-Output 'DRV8873 left-direction contract passed.'
