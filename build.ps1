$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $zig = Join-Path $PSScriptRoot 'tools\zig-x86_64-windows-0.17.0\zig.exe'
    $sdl = Join-Path $PSScriptRoot 'tools\SDL3-3.4.18\i686-w64-mingw32'
    New-Item -ItemType Directory -Path build -Force | Out-Null
    & $zig cc -target x86-windows-gnu -O2 -Wall -Wextra -shared src/proxy.c src/detection_hook.c src/prompt_hook.c src/xinput1_3.def -I "$sdl\include" "$sdl\lib\libSDL3.dll.a" -o build/xinput1_3.dll
    if ($LASTEXITCODE) { throw 'DLL build failed' }
    & $zig cc -target x86-windows-gnu -O2 -Wall -Wextra src/check.c -I "$sdl\include" -o build/controller-check.exe
    if ($LASTEXITCODE) { throw 'Diagnostic build failed' }
    Copy-Item -LiteralPath "$sdl\bin\SDL3.dll" -Destination build/SDL3.dll -Force
} finally { Pop-Location }
