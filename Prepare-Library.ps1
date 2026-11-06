param(
    [string]$Source = "$PSScriptRoot\content.json",
    [string]$Version = (Get-Date -Format 'yyyy.MM.dd.HHmmss'),
    [string]$OutputDirectory = "$PSScriptRoot\library"
)
$ErrorActionPreference = 'Stop'
$raw = Get-Content -LiteralPath $Source -Raw -Encoding UTF8
if (!$raw.TrimStart().StartsWith('[')) { throw 'Sursa trebuie sa fie o lista JSON.' }
$entries = ConvertFrom-Json -InputObject $raw
if (!$entries.Count) { throw 'Setul este gol.' }
$ids = @{}
foreach ($entry in $entries) {
    if ($null -eq $entry.id -or $entry.id -lt 1 -or $entry.id -gt [int]::MaxValue -or [double]$entry.id -ne [Math]::Floor([double]$entry.id) -or $ids.ContainsKey([int]$entry.id)) { throw "ID invalid sau duplicat: $($entry.id)" }
    $ids[[int]$entry.id] = $true
    foreach ($field in @('en','ro','it','meaning','exampleEn','exampleRo','exampleIt','kind')) {
        if ($entry.$field -isnot [string] -or [string]::IsNullOrWhiteSpace($entry.$field) -or $entry.$field.Length -gt 5000) { throw "Intrarea $($entry.id): camp invalid $field" }
    }
    if ($entry.status -ne 'approved') { throw "Intrarea $($entry.id) nu este aprobata. Adauga status: approved numai dupa verificare." }
    foreach ($field in @('level','topic','origin','reviewedBy')) { if ([string]::IsNullOrWhiteSpace($entry.$field)) { throw "Intrarea $($entry.id): lipseste $field" } }
}
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$json = '[' + (($entries | ForEach-Object { ConvertTo-Json -InputObject $_ -Depth 12 }) -join ',') + ']'
$utf8 = New-Object Text.UTF8Encoding($false)
$bytes = $utf8.GetBytes($json)
if ($bytes.Length -gt 8MB) { throw 'Setul depaseste 8 MB.' }
$sha = [Security.Cryptography.SHA256]::Create()
try { $hash = [BitConverter]::ToString($sha.ComputeHash($bytes)).Replace('-','').ToLowerInvariant() } finally { $sha.Dispose() }
$filename = "content-$hash.json"
[IO.File]::WriteAllBytes((Join-Path $OutputDirectory $filename),$bytes)
$manifest = @{ schemaVersion=1; version=$Version; url=$filename; sha256=$hash; count=$entries.Count } | ConvertTo-Json
[IO.File]::WriteAllText((Join-Path $OutputDirectory 'manifest.json'),$manifest,$utf8)
Write-Output "Pregatite $($entries.Count) intrari. Publica intai $filename, apoi manifest.json."
