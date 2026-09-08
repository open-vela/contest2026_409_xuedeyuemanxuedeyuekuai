$port = New-Object System.IO.Ports.SerialPort('COM3',1000000,[System.IO.Ports.Parity]::None,8,[System.IO.Ports.StopBits]::One)
$port.Open()
Start-Sleep -Milliseconds 300
$null = $port.ReadExisting()
# reboot into a clean state, then wait for NSH
$port.RtsEnable = $true; Start-Sleep -Milliseconds 120; $port.RtsEnable = $false
Start-Sleep -Seconds 5
$null = $port.ReadExisting()
$port.Write("lvgldemo widgets`r`n")
$buf = ''
$sw = [Diagnostics.Stopwatch]::StartNew()
while ($sw.Elapsed.TotalSeconds -lt 14) {
    $buf += $port.ReadExisting()
    Start-Sleep -Milliseconds 100
}
$port.Write([char]3)   # Ctrl+C back to shell
Start-Sleep -Milliseconds 800
$buf += $port.ReadExisting()
$port.Close()
[IO.File]::WriteAllText('C:\Users\21561\Desktop\比赛\lvgldemo_out.log', $buf)
$clean = ($buf -split "`n") | Where-Object { $_.Trim() }
$clean | Select-Object -First 60
