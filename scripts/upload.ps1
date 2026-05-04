param(
    [switch]$Upload # if set, upload the rock to LuaRocks
)

if(!(Test-Path "./project.yaml")) {
    Write-Host "[Error] project.yaml not found!" -ForegroundColor Red
    exit 1
}

try {
    $yamlRaw = Get-Content -Path "project.yaml" -Raw
    $config = $yamlRaw | ConvertFrom-Yaml
} catch {
    Write-Host "[Error] Failed to parse project.yaml" -ForegroundColor Red
    exit 1
}

$projectName = $config.project.name
$version     = $config.project.version
$rockRev     = "1"
$fullVersion = "$version-$rockRev"

if ([string]::IsNullOrEmpty($projectName)) {
    Write-Host "[Error] Metadata missing in project.yaml" -ForegroundColor Red
    exit 1
}

Write-Host "--- LuaRocks: $projectName $fullVersion ---" -ForegroundColor Magenta

# update rockspec version and rename
$oldRockspec = Get-ChildItem -Filter "*.rockspec" | Select-Object -First 1
$newRockspecName = "$projectName-$fullVersion.rockspec"

Write-Host "[1/2] Updating rockspec version and filename..." -ForegroundColor Gray
if ($oldRockspec) {
    $content = Get-Content $oldRockspec.FullName -Raw
    $content = $content -replace 'version\s*=\s*".*"', "version = `"$fullVersion`""
    $content | Set-Content $oldRockspec.FullName

    if ($oldRockspec.Name -ne $newRockspecName) {
        Rename-Item -Path $oldRockspec.FullName -NewName $newRockspecName -Force
    }
}

# install locally
Write-Host "[2/2] Installing rock locally..." -ForegroundColor Gray
luarocks make $newRockspecName --local

if ($Upload) {
    Write-Host "--- Uploading to LuaRocks.org ---" -ForegroundColor Cyan
    luarocks upload $newRockspecName
}

Write-Host "--- LuaRocks Task Completed ---" -ForegroundColor Green
