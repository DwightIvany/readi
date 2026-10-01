@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" >nul
if errorlevel 1 exit /b 1
python -m cmake -B build-ninja -S vst -G Ninja -DCMAKE_BUILD_TYPE=Release || exit /b 1
python -m cmake --build build-ninja || exit /b 1
python -m cmake --build build-ninja --target test || exit /b 1
