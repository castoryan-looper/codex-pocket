param([string]$ArduinoCli='arduino-cli',[ValidateSet('c6-epaper','taiji-knob32')][string]$Device='c6-epaper',[string]$ConfigFile='')
$ErrorActionPreference='Stop'
$cliArgs=@()
if($ConfigFile){$cliArgs+=@('--config-file',$ConfigFile)}
if($Device -eq 'taiji-knob32'){
  # Stage shared application plus only this board's hardware ports. Source files
  # remain single-copy; never pull private device_config.h into build artifacts.
  $stage=Join-Path $PSScriptRoot 'build/taiji-knob32/source/Pocket'
  New-Item -ItemType Directory -Force -Path $stage | Out-Null
  Get-ChildItem (Join-Path $PSScriptRoot 'firmware/Pocket') -File | Where-Object {$_.Extension -in '.ino','.h' -and $_.Name -ne 'device_config.h'} | Copy-Item -Destination $stage -Force
  Copy-Item (Join-Path $PSScriptRoot 'firmware/Pocket/port_adc.cpp') -Destination $stage -Force
  Get-ChildItem (Join-Path $PSScriptRoot 'firmware/common') -File | Copy-Item -Destination $stage -Force
  Get-ChildItem (Join-Path $PSScriptRoot 'firmware/boards/taiji_knob32') -File | Where-Object {$_.Extension -in '.h','.c','.cpp'} | Copy-Item -Destination $stage -Force
  & $ArduinoCli @cliArgs compile --fqbn 'esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB' --build-property 'compiler.cpp.extra_flags=-DPOCKET_TAIJI_KNOB32=1' --libraries (Join-Path $PSScriptRoot 'firmware/libraries') --build-path (Join-Path $PSScriptRoot 'build/taiji-knob32/app') $stage
}else{
  & $ArduinoCli @cliArgs compile --fqbn 'esp32:esp32:esp32c6:CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB' --libraries (Join-Path $PSScriptRoot 'firmware/libraries') --build-path (Join-Path $PSScriptRoot 'build/c6-epaper') (Join-Path $PSScriptRoot 'firmware/Pocket')
}
if ($LASTEXITCODE -ne 0) { throw 'Firmware compilation failed' }
