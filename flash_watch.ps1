param(
    [string]$Port = 'COM4',
    [string]$Image = (Join-Path $PSScriptRoot 'nuttx_huangshan.bin')
)

$sftool = Join-Path $env:USERPROFILE '.sifli\tools\sftool\0.2.5\sftool.exe'

if (-not (Test-Path -LiteralPath $Image)) {
    throw "Firmware not found: $Image"
}

& $sftool `
    -c SF32LB52 `
    -p $Port `
    -b 1000000 `
    --before default_reset `
    --after soft_reset `
    write_flash "$Image@0x12010000"

exit $LASTEXITCODE
