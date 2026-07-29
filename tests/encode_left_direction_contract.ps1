$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$source = Get-Content -Raw (Join-Path $root 'core\src\encode.c')

if ($source -notmatch '(?s)channel\s*==\s*ENCODE_LEFT\s*\)\s*\{\s*delta\s*=\s*-delta\s*;') {
    throw 'Left encoder feedback must invert the AB1 count to match the forward wheel convention.'
}

Write-Output 'Left encoder direction contract passed.'
