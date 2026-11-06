$ErrorActionPreference = 'Stop'
$qtRoot = 'C:\Qt\6.12.0\mingw_64'
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$qtRoot\bin;$env:PATH"
cmake -S $PSScriptRoot -B "$PSScriptRoot\build-qt" -G Ninja "-DCMAKE_PREFIX_PATH=$qtRoot" -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE) { throw 'Configurare esuata' }
cmake --build "$PSScriptRoot\build-qt" --target LimbiPublisher ContentTests
if ($LASTEXITCODE) { throw 'Compilare esuata' }
$destination = "$PSScriptRoot\dist\LimbiPublisher"
New-Item -ItemType Directory -Force $destination | Out-Null
Copy-Item "$PSScriptRoot\build-qt\LimbiPublisher.exe" $destination
Copy-Item "$PSScriptRoot\library-source.json","$PSScriptRoot\PUBLISHER.md" $destination
& "$qtRoot\bin\windeployqt.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw "$destination\LimbiPublisher.exe"
if ($LASTEXITCODE) { throw 'Impachetare esuata' }
Compress-Archive -Path $destination -DestinationPath "$PSScriptRoot\dist\LimbiPublisher-Windows.zip" -Force
