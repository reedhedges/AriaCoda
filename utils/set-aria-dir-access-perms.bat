@echo off
echo Giving local accounts in the Users group write access to ARIA installation directory.
echo This allows you to more easily recompile or modify ARIA and components as needed.
echo Open ARIA directory properties to change if desired.
if "%*" == "" (
  set ARIADIR=C:\Program Files\MobileRobots\Aria
) else (
  echo Using ARIA directory specified as script argument: %*
  set ARIADIR=%*
)
@echo on
icacls "%ARIADIR%" /T /grant Users:F
REM @pause
