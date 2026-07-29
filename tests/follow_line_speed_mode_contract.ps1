$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$source = Get-Content -Raw (Join-Path $root 'core\src\follow_line.c')
$update = [regex]::Match($source, '(?s)float\s+FollowLine_Update\s*\([^\)]*\)\s*\{.*?\n\}')

if (-not $update.Success) {
    throw 'follow_line.c must define FollowLine_Update.'
}

if ($update.Value -notmatch 'MG513XGMR_Set_Speed\s*\(\s*MG513XGMR_LEFT\s*,') {
    throw 'FollowLine_Update must set the left wheel through MG513XGMR_Set_Speed.'
}

if ($update.Value -notmatch 'MG513XGMR_Set_Speed\s*\(\s*MG513XGMR_RIGHT\s*,') {
    throw 'FollowLine_Update must set the right wheel through MG513XGMR_Set_Speed.'
}

if ($update.Value -match 'g_motor_(left|right)\.target_speed\s*=') {
    throw 'FollowLine_Update must not directly assign target_speed while motor mode can remain STOP.'
}

Write-Output 'Follow-line speed-mode contract passed.'
