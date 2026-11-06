$ErrorActionPreference = 'Stop'
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
if (!(Test-Path $compiler)) { $compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework\v4.0.30319\csc.exe' }
Push-Location $PSScriptRoot
try {
    & $compiler /nologo /target:winexe /optimize+ /out:Limbi.exe /reference:System.Windows.Forms.dll /resource:Limbi.ps1,Limbi.ps1 /resource:content.json,content.json /resource:Start-Limbi.vbs,Start-Limbi.vbs Launcher.cs
    if ($LASTEXITCODE -ne 0) { throw 'Compilarea a esuat.' }
    Write-Output 'Creat: Limbi.exe'
} finally { Pop-Location }
