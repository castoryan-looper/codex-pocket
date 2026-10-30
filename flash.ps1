param([Parameter(Mandatory=$true)][string]$Port,[string]$Python='python',[ValidateSet('c6-epaper','taiji-knob32')][string]$Device='c6-epaper')
$ErrorActionPreference='Stop'
$image=Join-Path $PSScriptRoot "build/$Device/app/Pocket.ino.merged.bin"
if($Device -eq 'c6-epaper'){$image=Join-Path $PSScriptRoot 'build/c6-epaper/Pocket.ino.merged.bin'}
if (-not (Test-Path -LiteralPath $image)) { throw 'Build the firmware first with build.ps1' }
$chip=if($Device -eq 'taiji-knob32'){'esp32s3'}else{'esp32c6'}
$after=if($Device -eq 'taiji-knob32'){'no-reset'}else{'hard-reset'}
& $Python -m esptool --chip $chip --port $Port --baud 921600 --after $after write-flash 0 $image
if ($LASTEXITCODE -ne 0) { throw 'Firmware flash failed' }
if($Device -eq 'taiji-knob32'){
  & $Python (Join-Path $PSScriptRoot 'tools/start_s3_application.py') --port $Port
  if($LASTEXITCODE -ne 0){throw 'Firmware written, but S3 application start needs attention'}
}
