$ErrorActionPreference='Stop'
Set-Location -LiteralPath $PSScriptRoot
$python=Join-Path $PSScriptRoot '.venv/Scripts/python.exe'
if (-not (Test-Path -LiteralPath $python)) {
    python -m venv .venv
    & $python -m pip install -r bridge/requirements.txt
    if ($LASTEXITCODE -ne 0) { throw 'Python dependencies failed' }
}
& $python bridge/bridge.py --open
