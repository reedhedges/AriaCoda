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
call .\PATHMGR.CMD /backup /user /y %USERPROFILE%\user_path_backup
call .\PATHMGR.CMD /backup /system  /y %USERPROFILE\system_path_backup
call .\PATHMGR.CMD /del /all /y "%ARIABINPATH%"
call .\PATHMGR.CMD /del /all /y "%ARIAPATH%\python"
pause

