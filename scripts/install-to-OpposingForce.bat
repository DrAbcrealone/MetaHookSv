@echo off

:: Check if SolutionDir is already set and non-empty
if not defined SolutionDir (
    :: Only set SolutionDir if it's not already set
    SET "SolutionDir=%~dp0.."
)

:: Ensure the path ends with a backslash
if not "%SolutionDir:~-1%"=="\" SET "SolutionDir=%SolutionDir%\"

cd /d "%SolutionDir%"

if not exist "%~dp0Build\svencoop.exe" goto fail_nobuild

set LauncherExe=metahook.exe
set LauncherMod=gearbox
set FullGameName=Half-Life : Opposing Force
set ShortGameName=OpposingForce

for /f "delims=" %%a in ('"tools\SteamAppsLocation" 50 InstallDir') do set GameDir=%%a

if "%GameDir%"=="" goto fail

echo -----------------------------------------------------

echo Copying files...

set GameSDL2Path=%GameDir%\SDL2.dll
for /f "delims=" %%i in ('powershell.exe -Command "$filePath = '%GameSDL2Path%'; $fileVersion = [System.Diagnostics.FileVersionInfo]::GetVersionInfo($filePath).FileVersion; Write-Output $fileVersion"') do set "GameSDL2_fileVersion=%%i"

if "%GameSDL2_fileVersion%"=="2, 0, 20, 0" (
    echo SDL2 version is "%GameSDL2_fileVersion%", no need to replace SDL2
    goto :no_replace_sdl2
)

if "%GameSDL2_fileVersion%"=="2, 0, 16, 0" (
    echo SDL2 version is "%GameSDL2_fileVersion%", no need to replace SDL2
    goto :no_replace_sdl2
)

echo SDL2 version is "%GameSDL2_fileVersion%", need to replace SDL2
copy "%~dp0Build\SDL2.dll" "%GameDir%\" /y
goto :no_replace_sdl2

:no_replace_sdl2


copy "%~dp0Build\svencoop.exe" "%GameDir%\%LauncherExe%" /y
copy "%~dp0Build\FreeImage.dll" "%GameDir%\" /y
xcopy "%~dp0Build\svencoop" "%GameDir%\%LauncherMod%" /y /e
xcopy "%~dp0Build\valve" "%GameDir%\%LauncherMod%" /y /e
xcopy "%~dp0Build\gearbox" "%GameDir%\%LauncherMod%" /y /e

if not exist "%GameDir%\%LauncherMod%\metahook\configs\plugins.lst" copy "%GameDir%\%LauncherMod%\metahook\configs\plugins_goldsrc.lst" "%GameDir%\%LauncherMod%\metahook\configs\plugins.lst" /y

del "%GameDir%\%LauncherMod%\metahook\configs\plugins_goldsrc.lst"
del "%GameDir%\%LauncherMod%\metahook\configs\plugins_svencoop.lst"
del "%GameDir%\%LauncherMod%\metahook\configs\plugins_svencoop_avx2.lst"

powershell $shell = New-Object -ComObject WScript.Shell;$shortcut = $shell.CreateShortcut(\"MetaHook for %ShortGameName%.lnk\");$shortcut.TargetPath = \"%GameDir%\%LauncherExe%\";$shortcut.WorkingDirectory = \"%GameDir%\";$shortcut.Arguments = \"-insecure -game %LauncherMod%\";$shortcut.Save();

echo -----------------------------------------------------

echo Make sure that you have all plugins you want in the plugins.lst

notepad "%GameDir%\%LauncherMod%\metahook\configs\plugins.lst"

echo Done
echo Please launch game from shortcut "MetaHook for %ShortGameName%"
pause
exit

:fail

echo Failed to locate GameInstallDir of %FullGameName%, please make sure Steam is running and you have %FullGameName% installed correctly.
pause
exit

:fail_nobuild

echo Compiled binaries not found ! You have to download compiled zip from github release page or compile the sources by yourself before installing !!!
pause
exit