@echo off


REM Force use of the same compiler as used to build ChimeraX
@REM call "%VS170COMNTOOLS%"\vcvars64.bat

set VCINSTALLDIR=C:\Program Files\Microsoft Visual Studio\2022\Community\VC
echo VCINSTALLDIR: %VCINSTALLDIR%
call "%VCINSTALLDIR%\Auxiliary\Build\vcvars64.bat"

@REM projects
set COO_PROJECT=dde-cooperation
set DT_PROJECT=data-transfer

REM defaults - override them by creating a build_env.bat file
set B_BUILD_TYPE=Release
set B_QT_ROOT=C:\Qt
set B_QT_VER=5.15.2
set B_QT_MSVC=msvc2019_64
set B_BONJOUR=%~dp0\3rdparty\ext\BonjourSDK

if "%OPENSSL_ROOT_DIR%"=="" (
    set OPENSSL_ROOT_DIR=C:\Program Files\OpenSSL-Win64
)

set savedir=%cd%
cd /d %~dp0

REM cmake generator name for the target build system
if "%VisualStudioVersion%"=="15.0" (
    set cmake_gen=Visual Studio 15 2017
) else if "%VisualStudioVersion%"=="16.0" (
    set cmake_gen=Visual Studio 16 2019
) else (
    echo Visual Studio version was not detected: %VisualStudioVersion%
    echo Did you forget to run inside a VS developer prompt?
    echo Using the default cmake generator.
    set cmake_gen=Visual Studio 17 2022
)

if exist build_env.bat call build_env.bat

REM full path to Qt stuff we need
set B_QT_FULLPATH=%B_QT_ROOT%\%B_QT_VER%\%B_QT_MSVC%

echo Qt: %B_QT_FULLPATH%

rmdir /q /s build
mkdir build
if ERRORLEVEL 1 goto failed
cd build
mkdir installer-inno

echo ------------starting cmake------------

cmake -G "%cmake_gen%" -A x64 -D CMAKE_BUILD_TYPE=%B_BUILD_TYPE% -D CMAKE_PREFIX_PATH="%B_QT_FULLPATH%" -D QT_VERSION=%B_QT_VER% ..
if ERRORLEVEL 1 goto failed
cmake --build . --config %B_BUILD_TYPE%
if ERRORLEVEL 1 goto failed
if exist output\%B_BUILD_TYPE% (
    copy output\%B_BUILD_TYPE%\* output\%COO_PROJECT%\%B_BUILD_TYPE%\ > NUL
    del output\%COO_PROJECT%\%B_BUILD_TYPE%\quazip5.* > NUL
    copy "%OPENSSL_ROOT_DIR%\libcrypto-1_1-x64.dll" output\%COO_PROJECT%\%B_BUILD_TYPE%\ > NUL
    copy "%OPENSSL_ROOT_DIR%\libssl-1_1-x64.dll" output\%COO_PROJECT%\%B_BUILD_TYPE%\ > NUL
    
    mkdir installer-inno\%COO_PROJECT%
    copy "%B_BONJOUR%\Bonjour64.msi" installer-inno\%COO_PROJECT%\ > NUL
    if exist output\%COO_PROJECT%\%B_BUILD_TYPE%\vc_redist.x64.exe (
        move output\%COO_PROJECT%\%B_BUILD_TYPE%\vc_redist.x64.exe installer-inno\%COO_PROJECT%\ > NUL
    )


    copy output\%B_BUILD_TYPE%\* output\%DT_PROJECT%\%B_BUILD_TYPE%\ > NUL
    del output\%DT_PROJECT%\%B_BUILD_TYPE%\QtZeroConf.* > NUL
    copy "%OPENSSL_ROOT_DIR%\libcrypto-1_1-x64.dll" output\%DT_PROJECT%\%B_BUILD_TYPE%\ > NUL
    copy "%OPENSSL_ROOT_DIR%\libssl-1_1-x64.dll" output\%DT_PROJECT%\%B_BUILD_TYPE%\ > NUL
    mkdir installer-inno\%DT_PROJECT%
    if exist output\%DT_PROJECT%\%B_BUILD_TYPE%\vc_redist.x64.exe (
        move output\%DT_PROJECT%\%B_BUILD_TYPE%\vc_redist.x64.exe installer-inno\%DT_PROJECT%\ > NUL
    )
) else (
    echo Remember to copy supporting binaries and configuration files!
)

echo Build completed successfully

set INNO_ROOT=C:\Program Files (x86)\Inno Setup 6

echo Building 64-bit Windows installer...

"%INNO_ROOT%\ISCC.exe" /Qp %COO_PROJECT%-setup.iss
move %cd%\installer-inno\deepin-cooperation-* %cd%\installer-inno\%COO_PROJECT%\ > NUL
if ERRORLEVEL 1 goto issfailed
"%INNO_ROOT%\ISCC.exe" /Qp deepin-%DT_PROJECT%-setup.iss
move %cd%\installer-inno\deepin-datatransfer-* %cd%\installer-inno\%DT_PROJECT%\ > NUL
if ERRORLEVEL 1 goto issfailed

echo Build all Windows installer successfully!!!

@REM echo ------------cmake again forgenerate sources------------
@REM cmake -G "%cmake_gen%" -A x64 -D CMAKE_BUILD_TYPE=%B_BUILD_TYPE% -D CMAKE_PREFIX_PATH="%B_QT_FULLPATH%" -D QT_VERSION=%B_QT_VER% ..

set BUILD_FAILED=0
goto done

:issfailed
echo Make Windows installer failed

:failed
set BUILD_FAILED=%ERRORLEVEL%
echo Build failed

:done
cd /d %savedir%

set B_BUILD_TYPE=
set B_QT_ROOT=
set B_QT_VER=
set B_QT_MSVC=
set B_BONJOUR=
set B_QT_FULLPATH=
set savedir=
set cmake_gen=

set INNO_ROOT=

EXIT /B %BUILD_FAILED%
