param([Parameter(Mandatory=$true)][string]$Cloudflared)
$ErrorActionPreference='Stop'
$runtime=Join-Path $PSScriptRoot 'bridge/runtime'
New-Item -ItemType Directory -Force -Path $runtime | Out-Null
$log=Join-Path $runtime 'tunnel.log'
$outLog=Join-Path $runtime 'tunnel-stdout.log'
$process=Start-Process -FilePath $Cloudflared -ArgumentList @('tunnel','--url','http://127.0.0.1:8787','--no-autoupdate') -WindowStyle Hidden -PassThru -RedirectStandardError $log -RedirectStandardOutput $outLog
Write-Host 'Starting a temporary WSS tunnel. Stop this script to close it.'
try {
    $deadline=(Get-Date).AddSeconds(60)
    $hostname=$null
    while ((Get-Date) -lt $deadline -and -not $process.HasExited) {
        if (Test-Path -LiteralPath $log) {
            $match=[regex]::Match((Get-Content -LiteralPath $log -Raw),'https://([a-z0-9-]+\.trycloudflare\.com)')
            if ($match.Success) { $hostname=$match.Groups[1].Value; break }
        }
        Start-Sleep -Milliseconds 500
    }
    if (-not $hostname) { throw "Tunnel unavailable; inspect $log" }
    $hostname | Set-Content -LiteralPath (Join-Path $runtime 'mobile-host.txt')
    Write-Host "Provision host: $hostname ; port: 443 ; TLS: enabled"
    Write-Host 'Use the root CA matching this endpoint. The hostname changes when restarted.'
    while (-not $process.HasExited) { Start-Sleep -Seconds 2 }
} finally {
    if (-not $process.HasExited) { Stop-Process -Id $process.Id }
}
