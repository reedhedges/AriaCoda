@echo off
if "%1" == "" (
  set ARIAPATH=C:\Program Files\MobileRobots\Aria
) else (
  set ARIAPATH=%1
)
if "%2" == "" (
  set ARIABINPATH=C:\Program Files\MobileRobots\Aria\bin
) else (
   set ARIABINPATH=%2
)
echo ARIAPATH=%ARIAPATH%
echo ARIABINPATH=%ARIABINPATH%
mkdir C:\TEMP
call pathmgr.cmd /backup /user /y %USERPROFILE%\user_path_backup
call pathmgr.cmd /backup /system  /y %USERPROFILE\system_path_backup
call pathmgr.cmd /add /all /y "%ARIABINPATH%"
call pathmgr.cmd /add /all /y "%ARIAPATH%\python"
call pathmgr.cmd /refresh 
echo ---
echo Done.
pause
