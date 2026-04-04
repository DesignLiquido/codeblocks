param(
    [Parameter(Mandatory = $true)]
    [string]$BinaryPath,

    [string]$OutputDir = "dist",

    [string]$Version = "0.1.0"
)

$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.IO.Compression.FileSystem

$root = Split-Path -Parent $PSScriptRoot
$binaryFullPath = (Resolve-Path $BinaryPath).Path
$outputFullPath = Join-Path $root $OutputDir

$pluginBaseName = "LinguagensDL"
$bundleName = "$pluginBaseName-$Version.cbplugin"
$resourceZipName = "$pluginBaseName.zip"

$manifestPath = Join-Path $root "recursos/manifest.xml"
$iconOnPath = Join-Path $root "recursos/LinguagensDL.png"
$iconOffPath = Join-Path $root "recursos/LinguagensDL-off.png"
$keywordsPath = Join-Path $root "recursos/palavras-chave"

$tempRoot = Join-Path $root (".tmp/package-plugin-" + [guid]::NewGuid().ToString("N"))
$resourceStage = Join-Path $tempRoot "resource-stage"
$bundleStage = Join-Path $tempRoot "bundle-stage"
New-Item -ItemType Directory -Path $resourceStage -Force | Out-Null
New-Item -ItemType Directory -Path $bundleStage -Force | Out-Null
New-Item -ItemType Directory -Path $outputFullPath -Force | Out-Null

Copy-Item -LiteralPath $manifestPath -Destination (Join-Path $resourceStage "manifest.xml")
$keywordsStagePath = Join-Path $resourceStage "palavras-chave"
New-Item -ItemType Directory -Path $keywordsStagePath -Force | Out-Null
Get-ChildItem -LiteralPath $keywordsPath | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination $keywordsStagePath -Recurse -Force
}

$resourceZipPath = Join-Path $bundleStage $resourceZipName
$resourceZipTemporaryPath = Join-Path $tempRoot "resource-bundle.zip"
if (Test-Path -LiteralPath $resourceZipPath)
{
    Remove-Item -LiteralPath $resourceZipPath -Force
}
if (Test-Path -LiteralPath $resourceZipTemporaryPath)
{
    Remove-Item -LiteralPath $resourceZipTemporaryPath -Force
}

[IO.Compression.ZipFile]::CreateFromDirectory(
    $resourceStage,
    $resourceZipTemporaryPath,
    [IO.Compression.CompressionLevel]::Optimal,
    $false)
Copy-Item -LiteralPath $resourceZipTemporaryPath -Destination $resourceZipPath -Force

$binaryDestinationPath = Join-Path $bundleStage "$pluginBaseName.dll"
Copy-Item -LiteralPath $binaryFullPath -Destination $binaryDestinationPath -Force
Copy-Item -LiteralPath $iconOnPath -Destination (Join-Path $bundleStage "LinguagensDL.png") -Force
Copy-Item -LiteralPath $iconOffPath -Destination (Join-Path $bundleStage "LinguagensDL-off.png") -Force

$bundlePath = Join-Path $outputFullPath $bundleName
$bundleTemporaryPath = Join-Path $tempRoot "plugin-bundle.zip"
if (Test-Path -LiteralPath $bundlePath)
{
    Remove-Item -LiteralPath $bundlePath -Force
}
if (Test-Path -LiteralPath $bundleTemporaryPath)
{
    Remove-Item -LiteralPath $bundleTemporaryPath -Force
}

[IO.Compression.ZipFile]::CreateFromDirectory(
    $bundleStage,
    $bundleTemporaryPath,
    [IO.Compression.CompressionLevel]::Optimal,
    $false)
Copy-Item -LiteralPath $bundleTemporaryPath -Destination $bundlePath -Force

Write-Host "Pacote gerado em: $bundlePath"
Remove-Item -LiteralPath $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
