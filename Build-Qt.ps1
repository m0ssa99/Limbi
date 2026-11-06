param([string]$PackageName = 'LimbiQt-0.3')
$ErrorActionPreference = 'Stop'
if ($PackageName -notmatch '^[A-Za-z0-9._-]+$') { throw 'Nume de pachet invalid' }
$qtRoot = 'C:\Qt\6.12.0\mingw_64'
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$qtRoot\bin;$env:PATH"
cmake -S $PSScriptRoot -B "$PSScriptRoot\build-qt" -G Ninja "-DCMAKE_PREFIX_PATH=$qtRoot" -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE) { throw 'Configurarea a esuat' }
cmake --build "$PSScriptRoot\build-qt"
if ($LASTEXITCODE) { throw 'Compilarea a esuat' }
$destination = "$PSScriptRoot\dist\$PackageName"
New-Item -ItemType Directory -Force $destination | Out-Null
Copy-Item "$PSScriptRoot\build-qt\LimbiQt.exe" $destination
& "$qtRoot\bin\windeployqt.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw "$destination\LimbiQt.exe"
if ($LASTEXITCODE) { throw 'Impachetarea a esuat' }
Compress-Archive -Path $destination -DestinationPath "$PSScriptRoot\dist\$PackageName-Windows.zip" -Force
