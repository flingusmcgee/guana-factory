# Running tests

This project contains a small test harness for the archetype loader.

If you use the project's usual build setup, add `tests/archetype_tests.cpp` to your build.

Quick local build (MinGW/g++ on Windows):

```powershell
mkdir -Force build
g++ -std=c++17 -I. -I./src -I./src/include tests/archetype_tests.cpp src/ArchetypeManager.cpp src/ArchetypeManager.cpp src/Log.cpp -o build/archetype_tests.exe
.
\build\archetype_tests.exe
```
Notes
- The test expects `res/archetypes` to contain `cube_base.archetype`, `cube_red.archetype`, and `cube_blue.archetype`.
- The test returns exit code 0 on success, non-zero on failure. Use this in CI.
