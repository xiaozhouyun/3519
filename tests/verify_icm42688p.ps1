$ErrorActionPreference = 'Stop'

$requiredFiles = @(
    'core/inc/icm42688p.h',
    'core/src/icm42688p.c'
)

foreach ($file in $requiredFiles) {
    if (-not (Test-Path -LiteralPath $file)) {
        throw "缺少 ICM-42688-P 驱动文件: $file"
    }
}

$header = Get-Content -LiteralPath 'core/inc/icm42688p.h' -Raw
$source = Get-Content -LiteralPath 'core/src/icm42688p.c' -Raw
$main = Get-Content -LiteralPath 'main.c' -Raw
$project = Get-Content -LiteralPath 'keil/empty_LP_MSPM0G3519_nortos_keil.uvprojx' -Raw

if ($header -notmatch 'ICM42688P_WHO_AM_I_VALUE\s*\(0x44U\)') { throw 'WHO_AM_I 期望值不是 0x44' }
if ($source -notmatch 'ICM42688P_REG_ACCEL_DATA_X1\s*\(0x1FU\)') { throw '加速度数据寄存器不是 0x1F' }
if ($source -notmatch 'ICM42688P_REG_GYRO_DATA_X1\s*\(0x25U\)') { throw '陀螺仪数据寄存器不是 0x25' }
if ($main -match 'imu660rc_') { throw 'main.c 仍在调用 IMU660RC' }
if ($main -notmatch 'icm42688p_init\(') { throw 'main.c 未初始化 ICM-42688-P' }
if ($project -notmatch 'icm42688p\.c') { throw 'Keil 工程未加入 icm42688p.c' }

Write-Host 'ICM-42686-P 静态集成检查通过'
