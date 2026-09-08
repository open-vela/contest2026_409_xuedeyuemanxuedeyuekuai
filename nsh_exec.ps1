param(
    [string]$Commands = "?",
    [string]$SerialPortName = 'COM4'
)
$port = New-Object System.IO.Ports.SerialPort($SerialPortName,1000000,[System.IO.Ports.Parity]::None,8,[System.IO.Ports.StopBits]::One)
$port.ReadTimeout = 200
$port.Open()
Start-Sleep -Milliseconds 300
$null = $port.ReadExisting()
foreach ($cmd in ($Commands -split ';')) {
    $port.Write(($cmd.Trim() + "`r`n"))
    Start-Sleep -Milliseconds 1500
    $buf += $port.ReadExisting()
}
$port.Close()
Write-Host $buf
