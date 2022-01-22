@echo off

REM Check for admin rights
net session >nul 2>&1
if not %errorLevel% == 0 (
	echo FAILED: Current permissions insufficient!
	echo FAILED: Administrator privileges are required for installation!
	PAUSE
	exit /b 1
)

cd %~dp0

if not exist "dep" mkdir dep
cd dep

rem ~~~~~~~~ GLFW Installation ~~~~~~~~~
if not exist "glfw3" git clone https://github.com/glfw/glfw.git glfw3
cd glfw3

if not exist "build" mkdir build
cd build
cmake ..
cmake --build . --target install --config Debug -DCMAKE_INSTALL_PREFIX:PATH=
cmake --build . --target install --config Release -DCMAKE_INSTALL_PREFIX:PATH=
cd ..
cd ..

rem ~~~~~~~~ GLM Installation ~~~~~~~~~
if not exist "glm" git clone https://github.com/g-truc/glm.git glm
cd glm

if not exist "build" mkdir build
cd build
cmake ..
cmake --build . --target install --config Debug
cmake --build . --target install --config Release
cd ..
cd ..

rem ~~~~~~~~ EDL Installation ~~~~~~~~~
if not exist "EngineDevelopmentLibrary" git clone https://github.com/Malience/EngineDevelopmentLibrary.git EngineDevelopmentLibrary
cd EngineDevelopmentLibrary

if not exist "build" mkdir build
cd build
cmake ..
cmake --build . --target install --config Debug
cmake --build . --target install --config Release
cd ..
cd ..

PAUSE