# gen_def.ps1 - generar forward.def para reenviar exports a d3d12_real.dll
$dumpbin = "dumpbin.exe"
$inputDll = ".\d3d12_real.dll"
$outDef = ".\forward.def"

if (-not (Test-Path $inputDll)) {
    Write-Error "No se encontró $inputDll. Asegúrate de haber copiado d3d12.dll -> d3d12_real.dll"
    exit 1
}

# Ejecuta dumpbin y captura la sección de EXPORTS
$lines = & $dumpbin /EXPORTS $inputDll 2>$null
$exports = @()
$start = $false
foreach ($line in $lines) {
    if ($line -match "ordinal") { $start = $true; continue }
    if (-not $start) { continue }
    $trim = $line.Trim()
    if ($trim -eq "") { continue }
    $parts = $trim -split "\s+"
    if ($parts.Length -ge 4) {
        $name = $parts[3]
        if ($name -ne "") { $exports += $name }
    }
}

$def = "LIBRARY d3d12.dll`r`nEXPORTS`r`n"
foreach ($e in $exports) {
    $def += "    $e=d3d12_real.$e`r`n"
}

Set-Content -Encoding ASCII $outDef $def
Write-Host "Generated $outDef with $($exports.Count) exports."
