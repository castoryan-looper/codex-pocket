param([Parameter(Mandatory=$true)][string]$Port,[string]$Python='python',[ValidateSet('c6-epaper','taiji-knob32')][string]$Device='c6-epaper')
$ErrorActionPreference='Stop'
$image=Join-Path $PSScriptRoot "build/$Device/app/Pocket.ino.merged.bin"
if($Device -eq 'c6-epaper'){$image=Join-Path $PSScriptRoot 'build/c6-epaper/Pocket.ino.merged.bin'}
if (-not (Test-Path -LiteralPath $image)) { throw 'Build the firmware first with build.ps1' }
$chip=if($Device -eq 'taiji-knob32'){'esp32s3'}else{'esp32c6'}
& $Python -m esptool --chip $chip --port $Port --baud 921600 write-flash 0 $image
if ($LASTEXITCODE -ne 0) { throw 'Firmware flash failed' }
