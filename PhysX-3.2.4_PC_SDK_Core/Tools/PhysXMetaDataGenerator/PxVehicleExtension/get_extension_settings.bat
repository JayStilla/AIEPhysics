@echo off

rem Find SDK_ROOT by searching backwards from cwd for SDKs
set SDK_ROOT=%~p0
:sdkloop
if exist "%SDK_ROOT%\Source" goto :havesdkpath
set SDK_ROOT=%SDK_ROOT%..\
goto :sdkloop

:havesdkpath


call "%SDK_ROOT%\Tools\PhysXMetaDataGenerator\get_common_settings.bat"
set SRCPATH=PxVehicleExtensionAPI.h

if not defined TARGETDIR (set TARGETDIR="%SDK_ROOT%\Source\PhysXVehicle\src\PhysXMetaData")

set TARGETNAME=PxVehicle

set AUTOFILENAME1=%TARGETDIR%\include\%TARGETNAME%AutoGeneratedMetaDataObjectNames.h
set AUTOFILENAME2=%TARGETDIR%\include\%TARGETNAME%AutoGeneratedMetaDataObjects.h
set AUTOFILENAME3=%TARGETDIR%\src\%TARGETNAME%AutoGeneratedMetaDataObjects.cpp

echo. 2>%AUTOFILENAME1%
echo. 2>%AUTOFILENAME2%
echo. 2>%AUTOFILENAME3%

set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Include/vehicle"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/PhysXVehicle/src"