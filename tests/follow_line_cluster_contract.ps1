$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$source = Get-Content -Raw (Join-Path $root 'core\src\follow_line.c')
$calc = [regex]::Match($source, '(?s)float\s+FollowLine_Calc_Error\s*\([^\)]*\)\s*\{.*?\n\}')

if (-not $calc.Success) {
    throw 'follow_line.c must define FollowLine_Calc_Error.'
}

if ($calc.Value -notmatch 'cluster_count') {
    throw 'FollowLine_Calc_Error must group adjacent black sensors into clusters.'
}

if ($calc.Value -notmatch 'last_error') {
    throw 'FollowLine_Calc_Error must use the prior error to select between separate clusters.'
}

if ($calc.Value -match 'return\s*\(\s*weighted_sum\s*/\s*count\s*\)') {
    throw 'FollowLine_Calc_Error must not average all separated black-sensor clusters together.'
}

Write-Output 'Follow-line cluster-selection contract passed.'
