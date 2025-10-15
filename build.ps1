<#
Simple PowerShell build script for guana-factory.

Usage:
  .\build.ps1           # build only
  .\build.ps1 -Run      # build and run the executable
  .\build.ps1 -Clean    # remove build folder

This script expects g++ to be available on PATH (MSYS2 MinGW64 or similar).
#>

[CmdletBinding()]
param(
    [switch]$Run,
    [switch]$Clean,
    [string]$Target = "iguana.exe"
)

function Check-Gpp {
    $g = Get-Command g++ -ErrorAction SilentlyContinue
    return $null -ne $g
}

if ($Clean) {
    if (Test-Path build) {
        Write-Host "Removing build/ directory..."
        Remove-Item -Recurse -Force build
    } else {
        Write-Host "No build/ directory to remove."
    }
    exit 0
}

if (-not (Check-Gpp)) {
    Write-Error "g++ not found on PATH. Install MSYS2 MinGW-w64 or add g++ to PATH before building."
    exit 1
}

if (-not (Test-Path build)) { New-Item -ItemType Directory -Path build | Out-Null }

$srcFiles = @(
    'main.cpp',
    'src/Game.cpp',
    'src/Config.cpp',
    'src/EventManager.cpp',
    'src/EntityManager.cpp',
    'src/AssetManager.cpp',
    'src/PhysicsSystem.cpp',
    'src/CollisionSystem.cpp',
    'src/Log.cpp',
    'src/ArchetypeManager.cpp'
)

$includeArgs = '-Isrc -Isrc\include'
$cflags = '-std=c++23 -O1 -Wall -Wno-missing-braces'
$ldflags = '-Llib -lraylib -lopengl32 -lgdi32 -lwinmm'

$srcArg = $srcFiles -join ' '

$outPath = Join-Path -Path "build" -ChildPath $Target

Write-Host "Compiling -> $outPath"

$cmd = "g++ $cflags $includeArgs $srcArg -o $outPath $ldflags"
Write-Host $cmd

$proc = Start-Process -FilePath cmd.exe -ArgumentList "/c $cmd" -NoNewWindow -Wait -PassThru
if ($proc.ExitCode -ne 0) {
    Write-Error "Build failed (exit code $($proc.ExitCode)). See compiler output above."
    exit $proc.ExitCode
}

Write-Host "Build succeeded: $outPath"

if ($Run) {
    Write-Host "Running $outPath..."
    & $outPath
}
