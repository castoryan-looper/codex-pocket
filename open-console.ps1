$ErrorActionPreference='Stop'
$path=Join-Path $PSScriptRoot 'bridge/runtime/console-url.txt'
if (-not (Test-Path -LiteralPath $path)) { throw 'Run start.ps1 first' }
Start-Process (Get-Content -LiteralPath $path -Raw)
