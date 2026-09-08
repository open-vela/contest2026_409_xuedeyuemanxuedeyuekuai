param(
    [string]$SerialPortName = 'COM7',
    [int]$Seconds = 12
)
$port = New-Object System.IO.Ports.SerialPort($SerialPortName,1000000,[System.IO.Ports.Parity]::None,8,[System.IO.Ports.StopBits]::One)
$port.Open()
Start-Sleep -Milliseconds 300
$null = $port.ReadExisting()
$port.Write("reboot`r`n")
$buf = ''
$sw = [Diagnostics.Stopwatch]::StartNew()
while ($sw.Elapsed.TotalSeconds -lt $Seconds) {
    $buf += $port.ReadExisting()
    Start-Sleep -Milliseconds 200
}
$port.Close()
Write-Host $buf
