@echo off
REM ********************************************************************************
REM
REM pathmgr.cmd - cmd file that provides tools to add to, delete from, clean up, backup and restore
REM               the PATH environment variable either on a session only basis or 
REM               make the changes permanently in the registry.
REM               To get help type "pathmgr /?"
REM
REM Author: 	Pianoboy 
REM Date:   	04/04/2012
REM
REM Date:   	12/02/2013 
REM		Fix problem when no initial environment variable path or removing last path from
REM		environment variable		
REM
REM ********************************************************************************

setlocal enabledelayedexpansion
set "version=1.0.2"
:: Set default values here
:: scope used to indicate whether changes to user. system or both PATH variables 
set "scope=user"
:: If /x indicates that environment variables should be expanded. Default is blank
set "xpand=/"
:: If /r then a dscending order is usedto display a list. Default is nothing
set "order=/"
:: userPrompt :- /p(prompt for choice) | /y(no prompt answer yes)|/n (no prompt answer no)   
set "userPrompt=/p"
:: If /v then validate paths else /nv
set "validate=/v"
:: Name of this filepathmgr 
set "fileName=%~n0"
:: Name of this filepathmgr 
set "pathmgrPath=%~dp0"
:: Variable to receive input path 
set "fsPath="
set "action="
set "help=false"
set "addPathMgr=none"
:ARGUMENTS_LOOP
if "%~1"==""  ( goto SETVARIABLES) 
    :: Help switches
    if /I "%~1" == "/?" (set "help=true" & goto SHIFT)
    if /I "%~1" == "-?" (set "help=true" & goto SHIFT)
    :: Scope switches
    if /I "%~1" == "/user" (set "scope=user" & goto SHIFT )
    if /I "%~1" == "/system" (set "scope=system" & goto SHIFT )
    if /I "%~1" == "/all" (set "scope=all" & goto SHIFT)
    :: Action switches
    :: Default modifier for CLEAN is to keep all paths 
    if /I "%~1" == "/clean" (set "action=CLEAN" & set "validate=/nv" & goto SHIFT)
    if /I "%~1" == "/backup" (set "action=BACKUP" & goto SHIFT)
    if /I "%~1" == "/restore" (set "action=RESTORE" & goto SHIFT)
    if /I "%~1" == "/refresh" (set "action=REFRESH" & goto SHIFT)
    if /I "%~1" == "/path" (set "action=PATHMGR" & goto SHIFT)
    :: Default modifier for ADD is /v
    if /I "%~1" == "/add" (set "action=ADD" & set "validate=/v" & goto SHIFT)
    :: Default modifier for DELETE is /nv ie only delete the given path, /v will delete
    :: any invalid paths
    if /I "%~1" == "/delete" (set "action=DELETE" & set "validate=/nv" & goto SHIFT)
    if /I "%~1" == "/del" (set "action=DELETE" & set "validate=/nv" & goto SHIFT)
    if /I "%~1" == "/list" (set "action=LIST" & set "validate=/nv" & goto SHIFT)
    if /I "%~1" == "/env" (set "action=ENVIRONMENT" & set "validate=/nv" & goto SHIFT)
    if /I "%~1" == "/environment" (set "action=ENVIRONMENT" & set "validate=/nv" & goto SHIFT)
    :: Action modifier switches pathmgr /list
    if /I "%~1" == "/x" (set "xpand=/x" & goto SHIFT)
    if /I "%~1" == "/r" (set "order=/r" & goto SHIFT)
    if /I "%~1" == "/v" (set "validate=/v" & goto SHIFT) 
    if /I "%~1" == "/nv" (set "validate=/nv" & goto SHIFT) 
    if /I "%~1" == "/p" (set "userPrompt=/p" & goto SHIFT)
    if /I "%~1" == "/y" (set "userPrompt=/y" & goto SHIFT)
    if /I "%~1" == "/n" (set "userPrompt=/n" & goto SHIFT)
    if /I "%~1" == "/+" (set "addPathMgr=add" & goto SHIFT)
    if /I "%~1" == "/-" (set "addPathMgr=delete" & goto SHIFT)
    :: else parameter a path   
    set "fsPath=%~1"
       
    :: Make expansion safe by replacing % with ? 
    set "fsPath=!fsPath:^%%=?!"
    set "fsPath=!fsPath:%%=?!"

:SHIFT
Shift
goto ARGUMENTS_LOOP

:SETVARIABLES

:: Variables used by cal to GETREGVAL to retreive the path environment variable from the
:: appropriate user or system key.
set "keyUser=hkcu\environment"
set  "keySystem=hklm\system\currentcontrolset\control\session manager\environment"
set "value=path"

if "%help%" equ "true" goto HELP
:: If an action switch is provided then jump to goto location value
if "%action%" neq "" ( goto CALL_ACTION)
:: If no action is provided but a path is provided try to add the path,
:: else no parameters provided - display the error an provide help
if "%fsPath%" neq "" (set "action=ADD") else  (Call :ERROR "No valid parameters have been provided" & set "help=true" & goto HELP)

:CALL_ACTION
echo.
echo Action %action% pathmgr 
goto %action% 

:: LIST action - Call Getlist sub to display list
:LIST
call :GETLIST
goto EXIT


:: ENVIRONMENT Action list the current Path environment variable
:ENVIRONMENT
call :CLEAN_PATH listPath "%path%"  
call :LIST_PATH "%listPath%" %order% %validate%
goto EXIT


:: BACKUP action - call backup_path with a destination file path parameter.
:BACKUP
setlocal enabledelayedexpansion
call set "backupPath=!fsPath:?=%%!"
call :BACKUP_PATH "!backupPath!"
goto EXIT

:: RESTORE action - if fsPath is a valid file path call restore_path.
:RESTORE
echo Now restoring "%fsPath%"
setlocal enabledelayedexpansion
call set "backupPath=!fsPath:?=%%!"
if Not Exist "%backupPath%" ( call :ERROR "The file %fsPath% to restore does not exist?" & goto EXIT )
call :RESTORE_PATH "%backupPath%"
endlocal & set "newpath=%newPath%"
goto EXIT 

REM ********************************************************************************
REM 
REM REFRESH Action
REM Reset the PATH variable to the current registry value
REM
REM ********************************************************************************
:REFRESH
setlocal enabledelayedexpansion
call :GETREGVAL "%keyUser%" path userPath
call :GETREGVAL "%keySystem%" path systemPath
call :CLEAN_PATH userPath "%userPath%"
call :CLEAN_PATH systemPath "%systemPath%"
call set "userPath=!userPath:?=%%!"
call set "systemPath=!systemPath:?=%%!"
endlocal & set "newPath=%systemPath%%userPath%" 
echo PATH now refreshed
goto EXIT


REM ********************************************************************************
REM 
REM PATHMGR Action
REM Add the pathmgr path to PATH environment variable.
REM
REM ********************************************************************************
:PATHMGR
echo %filename% path is %pathmgrPath%
if "%addPathMgr%" equ "add" (set "fsPath=%pathMgrPath%" & goto ADD)
if "%addPathMgr%" equ "delete" (set "fsPath=%pathMgrPath%" & goto DELETE)
goto EXIT



REM ********************************************************************************
REM 
REM CLEAN action
REM Description: Will erase any leading or trailing spaces from path and will
REM 		 remove any duplicate paths.
REM 		 If scope is /user then duplicate paths in the user path variable will
REM		 be removed as well as paths that match paths in the system path.
REM              If the scope is /system then all duplicate paths in the system path
REM		 will be renmoved as well as matching paths in the user variable.
REM		 The /all switch will remove all duplicates.
REM		 Where an expandable environment variable in the path is expanded and matches
REM              a path the environment variable is retained and the expanded version of the the
REM              path is removed. 
REM  
REM ********************************************************************************
:CLEAN
echo Now cleaning environment path for %scope% users.
setlocal enabledelayedexpansion
:: regardless of whether user or system we require 
:: clean versions of both the user and system path.
call :GETREGVAL "%keyUser%" path userPath
call :CLEAN_PATH userPath "%userPath%" "%validate%"
call :GETREGVAL "%keySystem%" path systemPath
call :CLEAN_PATH systemPath "%systemPath%" "%validate%"
if "%scope%" equ "system" goto REMOVE_SYSTEM_DUPLICATES
call :REMOVE_DUPLICATE_PATHS userPath "%userPath%" 
if "%scope%" equ "user" goto USER_AND_SYSTEM

:REMOVE_SYSTEM_DUPLICATES
call :REMOVE_DUPLICATE_PATHS systemPath "%systemPath%" 

:USER_AND_SYSTEM
:: Remove duplicates in user path that are also in system path
call :REMOVE_DUPLICATE_PATHS userPath "%systemPath%" "%userPath%" 

call :CHOOSE_YN "Do you wish to make a permanent change to the registry Y/N?"
:: If no exit
if errorlevel 2 ( goto END_CLEAN )
:: Need to replace ? with %% in path in path so not expanded
call :UPDATE_REGISTRY "%keyUser%"  "!userPath:?=%%%%!" 
if "%scope%" equ "user" goto END_CLEAN
call :UPDATE_REGISTRY "%keySystem%" "!systemPath:?=%%%%!"
:END_CLEAN
:: Replace ? with % so path is expanded
:: Have decided better not to automatically change the path
set "userPath=!userPath:?=%%!"
set "systemPath=!systemPath:?=%%!"
endlocal & call set "newpath=%systemPath%%userPath%"
goto EXIT 




REM ********************************************************************************
REM
REM BACKUP_PATH
REM Description: Will backup paths to the file name provided as a parameter.
REM              The scope switch determines whether system, user or both paths are backed up.
REM              User paths are preceded with "USER PATH" and system paths are preceded with "SYSTEM PATH"
REM              in the output backup file. 
REM Arguments: backupPath - The full path name of the backup file.
REM
REM ********************************************************************************
:BACKUP_PATH
setlocal enabledelayedexpansion
set "folderPath=%~1"
if "%folderPath:~-1%" neq "\" set "folderPath=%folderPath%\"

:: If path is a folder path then no file name has been provided
if Exist "%folderPath%" (call :ERROR "The path %~1 is not a valid file name?" & exit /b)
:: If path not valid exit
if not exist %~dp1 (call :ERROR "The path %~dp1 does not exist?"  & exit /b)



echo Now backing up to '%~1'
REM First delete backup file.
erase  "%~1" 2>nul
if "%scope%" equ "system" (set "section=SYSTEM PATH" & goto BACKUP_SYSTEM)
call :GETREGVAL "%keyUser%" path listPath
call :CLEAN_PATH listPath "%listPath%"
echo USER PATH>> "%~1"
call :LIST_PATH "%listPath%" >> "%~1%"
if "%scope%" equ "user"  (exit /b)
:BACKUP_SYSTEM
call :GETREGVAL "%keySystem%" path listPath
call :CLEAN_PATH listPath "%listPath%"
echo SYSTEM PATH>> "%~1"
call :LIST_PATH "%listPath%" >> "%~1"
endlocal
exit /b


REM ********************************************************************************
REM
REM RESTORE_PATH
REM Description: Will read backup file line by line. If it encounters a "USER PATH"
REM              line it will restore following lines to the user path. If it encounters
REM              a "SYSTEM PATH" line it will restore all following lines to the system path.
REM              Note that /user, /system and /all switches have no effect. 
REM Arguments:   userpath - path to backup file
REM
REM ********************************************************************************
:RESTORE_PATH
setlocal enabledelayedexpansion
set "restorePath=%~1"
set "userPath="
set "systemPath="
:: Note change any environment variables marked with "%" to "?" to prevent expansion
for /f "usebackq tokens=*" %%a in ("%restorePath%") DO (	set "var=%%a"
							set "var=!var:%%=?!" 
							call :BUILD_PATH "!var!")
:: Exit if nothing to restore
if "%restore%" equ "" (echo Unable to restore %restorePath%  & exit /b)

:RESTORE_REGISTRY
set "user=false"
set "system=false"
:: need to clean path of trailing spaces - an artefact of redirecting list_path to file.
if "%userPath%" neq "" (set "userPath=!userPath: ;=;!" & set "user=true") else (
                        call :GETREGVAL "%keyUser%" path userPath
                        call :CLEAN_PATH userPath "!userPath!") 
if "%systemPath%" neq "" (set "systemPath=!systemPath: ;=;!" & set "system=true") else (
                        call :GETREGVAL "%keySystem%" path systemPath
			call :CLEAN_PATH systemPath "!systemPath!" )


:: offer choice to save to registry
call :CHOOSE_YN "Do you wish to make a permanent change to the registry Y/N?"
:: If no exit
if errorlevel 2 (goto END_RESTORE )
:: Need to replace ? with %% in path in path so not expanded
if "%user%" equ "true" (call :UPDATE_REGISTRY "%keyUser%"  "!userPath:?=%%%%!" ) 
if "%system%" equ "true" (call :UPDATE_REGISTRY "%keySystem%" "!systemPath:?=%%%%!") 

:END_RESTORE
:: Replace ? with % so path is expanded
set "userPath=!userPath:?=%%!"
set "systemPath=!systemPath:?=%%!"
endlocal & call set "newPath=%systemPath%%userPath%"
exit /b



REM ********************************************************************************
REM
REM BUILD_PATH
REM Description: Utility used by restore registry to buildup restore path 
REM
REM ********************************************************************************
:BUILD_PATH
if "%~1" equ "USER PATH" set "restore=user" & exit /b
if "%~1" equ "SYSTEM PATH" set "restore=system" & exit /b
if Not Defined restore (call :ERROR "The backup file format is not valid?" & exit /b)
set "pathVar=%~1"
if "%restore%" equ "user" set "userPath=%userPath%%pathVar%;"
if "%restore%" equ "system" set "systemPath=%systemPath%%pathVar%;"

exit /b


REM ********************************************************************************
REM
REM GETLIST
REM Description: Provides a list of paths in user or system or both paths
REM              according to the scope switch. If the xpand switch /x is used 
REM              paths will be expanded. If the order switch /r is used the list is in reverse order.
REM
REM ********************************************************************************
:GETLIST
setlocal
set "userPath="
set "systemPath="
set "heading=USER ^& SYSTEM PATH"

if "%scope%" equ "system" (set "heading=SYSTEM PATH" & goto LIST_SYSTEM)
call :GETREGVAL "%keyUser%" path listPath
call :CLEAN_PATH listPath "%listPath%" 

if "%scope%" equ "user"  (set "heading=USER PATH" & goto LIST_OUTPUT)
set "userPath=%listPath%"

:LIST_SYSTEM
call :GETREGVAL "%keySystem%" path listPath
call :CLEAN_PATH listPath "%listPath%"
set "listPath=%listPath%%userPath%"
:LIST_OUTPUT
echo %heading%
call :LIST_PATH "%listPath%" "%xpand%" "%order%" "%validate%"
exit /b


REM ********************************************************************************
REM
REM ADD action
REM Description: Add a new path to the environment variable (default user path)
REM              if that path does not already exist in either the system or user path.
REM              If path is a valid file path then add its path. This allows drag and drop of files
REM		 to be used as the source of a path.
REM
REM ********************************************************************************
:ADD

setlocal enabledelayedexpansion
set "userPath="
set "systemPath="



:: Need to make path expandable if ^% has been used
call set "xpandPath=!fsPath:?=%%!"
set "folderPath=%xpandPath%"
if "%folderPath:~-1%" neq "\" (call set "folderPath=%folderPath%\") 
:: If path is a folder path then no file name has been provided

:: do not validate if validate=/nv
if "%validate%" equ "/nv" goto IS_VALIDATED
:: If valid folder path IS_VALIDATED

if exist "%folderPath%" goto IS_VALIDATED
:: Can get valid folder path from a file path.
:: However if dealing with expandable paths this may not be possible
:: If fsPath not equal to xpandPath then we have an expandable path
if "%fsPath%" neq "%xpandPath%" (call :ERROR "The folder path '%fsPath%' either does not exist or is invalid."
                            	 goto EXIT) 

:: Now test if valid file path exists by removing the trailing '\'
set "folderPath=%folderPath%###"
call set "folderPath=!folderPath:\###=!"
if not exist !folderPath! (call :ERROR "The folder path '%fsPath%' either does not exist or is invalid."
                            	 goto EXIT) 

:: Now get the path portion
call :GET_PATH "%xpandPath%" folderPath

if not exist "%folderPath%" (call :ERROR "The folder path '%fsPath%' either does not exist or is invalid."
                            goto EXIT) else (call set "fsPath=%folderPath%" )

call :CHOOSE_YN "Do you wish to add '%folderPath%' to the PATH Y/N?"
:: If no exitlist
if errorlevel 2 (goto EXIT )

:IS_VALIDATED

:: First check if path already exists in PATH environment variable
:: If it doesn't add it to PATH, however no messages as confusing if subsequently
:: also adding to registry  
call :CLEAN_PATH path "%path%"

call :PATH_FOUND "!fsPath!" "%path%" 

:: Path does not exist so add
if %errorlevel% equ 1 ( set "path=%path%!fsPath:?=%%!" & echo Path '!fsPath:?=%%!' added to PATH variable.) else (
                        echo PATH variable already contains !fsPath:?=%%!)

echo Now adding path '!fsPath:?=%%!' to the Path environment variable
call :GETREGVAL "%keyUser%" path userPath

call :GETREGVAL "%keySystem%" path systemPath

call :CLEAN_PATH userPath "%userPath%"

call :CLEAN_PATH systemPath "%systemPath%"

call :PATH_FOUND "!fsPath!" "%systemPath%%userPath%" 

:: Path already exists so finish up
if %errorlevel% equ 0 ( echo Registry already contains path !fsPath! &  goto END_ADD_PATH)


if "%scope%" equ "user" (set "userPath=%userPath%!fsPath!") else (set "systemPath=%systemPath%!fsPath!")

call :CHOOSE_YN "Do you wish to make permanent change to the registry Y/N?"
:: If no exit
if errorlevel 2 (goto END_ADD_PATH )
:: Now add to registry

:: Need to replace ? with %% in path in path so not expanded
if "%scope%" == "user" ( call :UPDATE_REGISTRY "%keyUser%"  "!userPath:?=%%%%!;" & echo Path '!fsPath:?=%%!' added to user registry PATH.) else (
			 call :UPDATE_REGISTRY "%keySystem%"  "!systemPath:?=%%%%!;" & echo Path '!fsPath:?=%%!' added to system registry PATH.)

:END_ADD_PATH
endlocal & call set "newpath=%path%"
goto EXIT  

REM ********************************************************************************
REM
REM DELETE action
REM Description: Delete a path from the environment variable.
REM              Will first look in user path, if not found will look
REM              in system path.
REM
REM ********************************************************************************
:DELETE
setlocal enabledelayedexpansion
if "%fsPath%" neq "" goto DEL_START 
:: Handle case where just delete no existant paths
if "%validate%" neq "/v" (call :ERROR "No path to delete or modifier switch /v missing." & goto EXIT)
call :CLEAN_PATH path "%path%" "%validate%
goto DEL_REG_ENTRIES

:DEL_START
set "userPath="
set "systemPath="

:: Need to determine if file has been dragged and dropped.
:: First decide whether a file path or a folder
:: If it ends with '\' then is folder  otherwise need to add '\' and
:: check again

if "%fsPath:~-1%" equ "\" (goto FIND_PATH) else (set "folderPath=%fsPath%\")

:: If is folder  can ignore 
if exist "%folderPath%" (goto FIND_PATH)
:: It is not a folder then check if it exists. If not may just be an invalid path so continue to find.
:: If it exists it is a file that has been dragged and dropped.
:: Thus need to extract the path otherwise simply continue
if not exist "%fsPath%" goto FIND_PATH
:: Now extract path
call ::GET_PATH "%fsPath%" fsPath

:FIND_PATH
echo Now removing path '!fsPath!' from the Path environment variable
:: First check if path already exists in PATH environment variable
:: If it doesn't add it to PATH, however no messages as confusing if subsequently
:: also adding to registry  
call :CLEAN_PATH path "%path%" "%validate%

call :PATH_FOUND "!fsPath!" "%path%" 

:: Path does  exist so delete
if %errorlevel% equ 0 ( set "path=!path:%foundPath%;=!")


:DEL_REG_ENTRIES
:: Delete registry entries

:: Need to make path expandable if ^% has been used
call set "xpandPath=!fsPath:?=%%!"

:: Need to retrieve both user and system path
call :GETREGVAL "%keyUser%" path userPath
call :CLEAN_PATH userPath "%userPath%" "%validate%
call :GETREGVAL "%keySystem%" path systemPath
call :CLEAN_PATH systemPath "%systemPath%" "%validate%

if "%fsPath%" equ "" goto DEL_REGISTRY 
call :PATH_FOUND "%fsPath%" "!userPath!" 
:: If errorlevel = 0 then path found delete from userpath
if %errorlevel% equ 0 (call set "userPath=!userPath:%foundPath%;=!" & set "scope=user" & goto DEL_REGISTRY)

call :PATH_FOUND "%fsPath%" "!systemPath!" 
if %errorlevel% equ 1 ( goto END_DEL_PATH)
call set "systemPath=!systemPath:%foundPath%;=!"
set "scope=system"
:DEL_REGISTRY
call :CHOOSE_YN "Do you wish to make permanent change to the registry Y/N?"
:: If no exit
if errorlevel 2 (goto END_DEL_PATH )
:: Now add to registry
echo userPath %userPath%
echo systemPath %systemPath%
:: Need to replace ? with %% in path in path so not expanded
if "%scope%" == "user" ( call :UPDATE_REGISTRY "%keyUser%"  "!userPath:?=%%%%!") else ( call :UPDATE_REGISTRY "%keySystem%"  "!systemPath:?=%%%%!")

:END_DEL_PATH

echo Path '%fsPath%' removed from Path environment variable.
endlocal & call set "newPath=%path%"
goto EXIT







REM ********************************************************************************
REM
REM REMOVE DUPLICATE PATHS
REM Description:	Will remove duplicate paths from path list.
REM Example:		call :REMOVE_DUPLICATE_PATHS "result variable name" "source path list" "target path list"
REM 			If no target path list will remove duplicates from the source path. 
REM 			If a target path is provided will remove any paths in target path list that match source paths.
REM
REM ********************************************************************************
:REMOVE_DUPLICATE_PATHS

setlocal enabledelayedexpansion
set "destinationName=%~1"
set "sourcePath=%~2" 
set targetPath=%sourcePath%
set  "targetSupplied=false"

if "%~3" neq "" ( set "targetPath=%~3" 
        set "targetSupplied=true")

:LOOP_REMOVE_DUPLICATES

:: Loop through each path in inputPath    
:: and set aPath to the path
   for /f "delims=;" %%a in ("%sourcePath%") do ( 
        set "aPath=%%a"

       
   )


set "targetPath=!targetPath:%aPath%;=!"

:: need to add back aPath to our targetPath
:: if targetSupplied is false as need to retain at least one instance
:: in file
if "%targetSupplied%" equ "false" set "targetPath=!targetPath!%aPath%;" 

:: if expandable need to expand 
:: note clean has replaced % with ? so must put back
:: Check if first character=?

if "%aPath:~0,1%" equ "?" ( call set "xpand=!aPath:?=%%!" ) else (goto CONTINUE)
set "targetPath=!targetPath:%xpand%;=!" 
set "sourcePath=!sourcePath:%xpand%;=!" 


:CONTINUE
set "sourcePath=!sourcePath:%aPath%;=!"

if "!sourcePath!" neq "" ( goto LOOP_REMOVE_DUPLICATES)

endlocal & set "%destinationName%=%targetPath%"

exit /b






goto EXIT

REM ********************************************************************************
REM
REM UTILITIES START HERE
REM
REM ********************************************************************************

REM ********************************************************************************
REM
REM CLEAN PATH
REM Description: Cleans up paths by removing any spaces and quotes and also
REM              will reset current path to match registry.
REM Parameters: sourcePathName	- The variable to return the result
REM 		sourcePath	- The path string to clean
REM             validate        - if /v will remove invalid paths or paths that don't exist
REM ********************************************************************************
:CLEAN_PATH
:: setlocal local variable only in scope until  endlocal called
:: EnableDelayedExpansion - a variable enclosed in "%" is evaluated at run time not at read time 
setlocal enabledelayedexpansion
set "sourcePathName=%1"
set "sourcePath=%~2"

REM Return if source path is empty
if "%sourcePath%" equ "" (set "sourcePath=" & exit /b)
if /i "%sourcePathName%" equ "systemPath" set "pathType=System Registry PATH value"
if /i "%sourcePathName%" equ "userPath" set "pathType=User Registry PATH value"
if /i "%sourcePathName%" equ "Path" set "pathType=Environment PATH variable"
if "%~3" equ "" (set "validate=/nv") else (set "validate=%~3")
:: Call to subroutine - need to remove any trailing spaces
:: as this will create a path of spaces
call :TRIM_RIGHT  sourcePath "%sourcePath%"
:: Call to subroutine - need to remove any leading spaces
:: as this will create a path of spaces
call :TRIM_LEFT  sourcePath "%sourcePath%"
:: Also need to make sure that last path ends with ;
:: so add ; if it does not.
if "%sourcePath:~-1%" neq ";" (
	set "sourcePath=%sourcePath%;"
)
 
rem set sourcepath=!sourcepath:"=!

:: if validation required 
if "%validate%" equ "/v" echo Checking for paths that don't exist in %pathType%.
:LOOP_CLEAN

:: Loop through each path in sourcePath    
:: and set aPath to the path
   for /f "delims=; tokens=1*" %%a in ("%sourcePath%") do ( 
        set "aPath=%%a"
        set "sourcePath=%%b" 

   )

::echo %aPath%
call :TRIM_RIGHT  aPath "!aPath!"
call :TRIM_LEFT  aPath "!aPath!"

:: if validation required 
if "%validate%" neq "/v" goto BUILD_CLEANPATH
:: if path exists continue with building up path else 

call set "xPath=!aPath:?=%%!"
if "!xPath:~-1!" neq "\" call set "xPath=!xPath!\"
if exist "!xPath!" (goto BUILD_CLEANPATH )else ( echo !xPath! does not exist & goto CHECK_LOOP_CONDITION)

:BUILD_CLEANPATH
if "%cleanPath%" equ "" (set "cleanPath=%aPath%;") else (set "cleanPath=!cleanPath!%aPath%;") 
:CHECK_LOOP_CONDITION
if "!sourcePath!" neq "" goto LOOP_CLEAN
::echo %cleanPath%
endlocal & call set %sourcePathName%=%cleanPath%
  
exit /b


REM *******************************************************************************************************
REM Trim character from end ofstring
REM Example call :TRIM_RCHAR VariableName "The String" character
REM *******************************************************************************************************
:TRIM_RCHAR
set "string=%~2###"
set char=%~3
setlocal enabledelayedexpansion
set "string=!string:%char%###=!"
endlocal & call set "string=%string%"
set %1=%string%
exit /b

REM *******************************************************************************************************
REM Trim from end of line
REM *******************************************************************************************************
:TRIM_RIGHT
set "string=%2"
:: Remove quotes
set string=###%string%###
set string=%string:"###=%
set string=%string:###"=%
set string=%string:###=%


:PROCESS_RIGHT_TRIM
if "%string:~-1%" equ " " ( 
       	set "string=%string:~0,-1%" 
        goto PROCESS_RIGHT_TRIM
       
) 

set %1=%string%
exit /b

REM *******************************************************************************************************
REM Trim from beginning of line
REM *******************************************************************************************************
:TRIM_LEFT
set "string=%2"
:: Remove quotes
set string=###%string%###
set string=%string:"###=%
set string=%string:###"=%
set string=%string:###=%
:PROCESS_LEFT_TRIM
if "%string:~0,1%" equ " " (
	set "string=%string:~1%" 
        goto PROCESS_LEFT_TRIM 
) 
set %1=%string%
exit /b

REM ********************************************************************************
REM
REM GETREGVAL
REM Description: will return the value of a registry key value. Will substitute
REM              a "?" for any "%" so that values are not automatically expanded.
REM Parameters: key - the key string  to find in the registry
REM 		value - the key value to return
REM		return - the name of environment variable to return the value.
REM
REM ********************************************************************************
:GETREGVAL
:: The key to query
set "key=%~1"
:: The value to find
set "value=%2"
:: The environment variable for return value
set "return=%3"

:: No. of words in value 
Set count=1
For  %%j in ("%value%") Do Set /A count+=1
setlocal enabledelayedexpansion
Set "regVal="
REM FOR /F "tokens=%count%*" %%A IN ('REG.EXE QUERY "%key%" /V %value% 2^> nul') DO (SET "regVal=%%B")
REM cannot pipe error to nul if we wish to detect error level
REM For loop seems to bury the error level must pipe error to file
:: Need to access reg twice - unable to access errorlevel in for loop?
:: No longer concerned with registry error
:: If registry corrupted ant path not discoverable - many other problems would exist
::REG.EXE QUERY "%key%" /V %value% > nul
::if errorlevel 1 exit /b 1
FOR /F "tokens=%count%*" %%A IN ('REG.EXE QUERY "%key%" /V %value% 2^> nul') DO ( SET "regVal=%%B")

:: Was going to check for valid entries in path
:: however, if not valid that is for user to decide.
:: Provide mechanism in clean to eliminate invalid entries.
:: substitute ? for % so that values not automatically expanded
set "regval=!regVal:%%=?!"
:: Remove quotes 
set regval=!regVal:"=!
if "%regval%" equ "%%=?" (set "regVal=")

endlocal & Set "%return%=%regVal%"

exit /b 0


REM *******************************************************************************************************
REM
REM UPDATE_REGISTRY
REM Description: Update the registry with the given value
REM 
REM *******************************************************************************************************
:UPDATE_REGISTRY
setlocal enabledelayedexpansion
:: replace trailing '\'
set "pathToUpdate=%~2"

REM check if empty path ie expand 
if "%pathToUpdate%" equ "?=%%" (set "pathToUpdate=")

if "%pathToUpdate:~-1%" equ "\" ( set "pathToUpdate=!pathToUpdate!###"
                                  set "pathToUpdate=!pathToUpdate:\###=!"        )



reg add "%~1" /v path /t REG_EXPAND_SZ /d "%pathToUpdate%" /f
:: No error action is necessary as GET_REGVAL always precedes UPDATE_REGISTRY
if errorlevel 1 (echo Unable to update path for registry key %1, try executing as administrator?) else (echo Registry key "%~1" updated successfully.)
exit /b



REM *******************************************************************************************************
REM
REM LIST PATH
REM Description: Lists path sorted in either ascending or descending order.
REM		 This is done by saving list to file and using sort on file.
REM Parameters: inputPath(required) - The path string to list
REM 		/x(optional) - 	If present will expand any environment variables in the 
REM				path before listing, the default is to leave paths unexpanded.
REM		/r(optional) -	If present will sort list in reverse order, the default is ascending order.
REM             /v(optional) -  If validate = /v then indicate 'does not exist'		
REM *******************************************************************************************************
:LIST_PATH
setlocal enabledelayedexpansion

set "mxpand=/"
set "morder=/"
set "mvalidate=/"
:ARGUMENTS_LIST_PATH
if "%~1"==""  ( goto START_LIST) 
    if "%~1" == "/" (goto SHIFT_LIST )
    if "%~1" == "/x" (set "mxpand=/x" & goto SHIFT_LIST )
    if "%~1" == "/r" (set "morder=/r" & goto SHIFT_LIST ) 
    if "%~1" == "/v" (set "mvalidate=/v" & goto SHIFT_LIST )
    :: else parameter a path if ends ?
    set "isPath=%~1"  
    if "%isPath:~-1%" == ";" (    set "inputPath=%~1" & goto SHIFT_LIST )


   
:SHIFT_LIST
Shift
goto ARGUMENTS_LIST_PATH
:START_LIST

:: Create new list file

:LOOP_LIST
:: Loop through each path in inputPath    
:: and set aPath to the path
   for /f "delims=; tokens=1*" %%a in ("%inputPath%") do ( 
        set "aPath=%%a"sort
        set "inputPath=%%b" 

   )
set "aPath=!aPath:?=%%!"
if "%mvalidate%" neq "/v" goto CHECK_EXPAND
call set "xpandPath=%aPath%"
if not exist "%xpandPath%" set "aPath=%aPath% does not exist" 

:CHECK_EXPAND
if "%mxpand%" equ "/x" call echo %aPath% >> %Temp%\pathlist.txt
if "%mxpand%" neq "/x" echo %aPath% >>  %Temp%\pathlist.txt
if "%inputPath%" neq "" goto LOOP_LIST
call sort %morder% <  %Temp%\pathList.txt
del  %Temp%\pathlist.txt
endlocal
exit /b

REM *******************************************************************************************************
REM
REM CHOOSE_YN
REM Description: Display simple yes/no choice to continue with action.
REM              If userPrompt = /p Display prompt.
REM          	 If userPrompt = /y Turn prompt off return yes
REM          	 If userPrompt = /nT urn prompt off return no
REM
REM Example:  Call :CHOOSE_YN "prompt string here" 
REM Return:  Yes = errorlevel 1, No = errorlevel 2
REM
REM *******************************************************************************************************
:CHOOSE_YN
:: Exit if prompt mode is /p
if "%userPrompt%" equ "/y" exit /b 1
if "%userPrompt%" equ "/n" exit /b 2
set /p "reply=%~1"

if /I "%reply%" equ "Y" (set "return=1") else (set "return=2")
rem choice /C YN /M "%~1"
exit /b %return%

REM *******************************************************************************************************
REM
REM PATH_FOUND
REM Description: Used to find if a folder path exists in current path
REM 		 If found errrlevel = 0 else if not found errorlevel = 1.
REM              The found value may be retreived in the variable foundPath.
REM *******************************************************************************************************
:PATH_FOUND
setlocal enabledelayedexpansion

set "pathVar=%~2"
REM if path variable is empty then no paths so no need to look any further 
if "%pathVar%" equ  "" (exit /b 1)

set "pathToFind=%~1"
:: if path does not end in slash make pathSlash '\'
if "%pathToFind:~-1%" neq "\" (set "pathSlash=\") else (set "pathSlash=")

:PATH_LOOP
for /f "delims=;" %%a in ("!pathVar!") do ( 
        set "aPath=%%a"
       
)
 
:: Remove aPath from path for next iteration
set "pathVar=!pathVar:%aPath%;=!"
call :TRIM_RIGHT  trimPath "!aPath!"
call :TRIM_LEFT  trimPath "!trimPath!"

:: if trimpath does not end in slash make trimSlash '\'
if "%trimPath:~-1%" neq "\" (set "trimSlash=\") else (set "trimSlash=")
rem echo %pathToFind%%pathSlash% %trimPath%%trimSlash%
:: if  path to match matches aPath 
if /I "%pathToFind%%pathSlash%" equ "%trimPath%%trimSlash%" (
	endlocal & call set "foundPath=%aPath%"
	exit /b 0) 

:: Now try expanding pathToFind
call set xpathToFind=!pathToFind:?=%%!
rem        echo !xpathToFind!%pathSlash% %trimPath%%trimSlash%
if /I "!xpathToFind:?=%%!%pathSlash%" equ "!trimPath!%trimSlash%"  (
	endlocal & call set "foundPath=%aPath%"
	exit /b 0) 

:: Finally try expanding trimPath
call set "trimPath=!trimPath:?=%%!"
rem    echo %pathToFind%%pathSlash% %trimPath%%trimSlash%
if /I "%pathToFind%%pathSlash%" equ "%trimPath%%trimSlash%"  (
	endlocal & call set "foundPath=%aPath%"
	exit /b 0) 


if "!pathVar!" neq "" goto PATH_LOOP
exit /b 1


REM *******************************************************************************************************
REM
REM ERROR
REM Description: Displays error message
REM Example:     call :ERROR "Message here"
REM
REM *******************************************************************************************************
:ERROR
echo.
echo %~1
exit /b

REM *******************************************************************************************************
REM
REM GET_PATH
REM Description: Returns a path given a file path
REM Example:     call :GET_PATH [path] [return variable name]
REM
REM *******************************************************************************************************
:GET_Path
set "%2=%~dp1"
exit /b

REM *******************************************************************************************************
REM
REM GET_FILE
REM Description: Returns a file name given a file path
REM Example:     call :GET_PATH [path] [return variable name]
REM
REM *******************************************************************************************************
:GET_FILE 
set "%2=%~n1"
exit /b

REM *******************************************************************************************************
REM
REM GET_FILENAME
REM Description: Returns a file name with extension given a file path
REM Example:     call :GET_PATH [path] [return variable name]
REM
REM *******************************************************************************************************
:GET_FILE 
set "%2=%~nx1"
exit /b



REM *******************************************************************************************************
REM
REM GET_EXT
REM Description: Returns a file extension given a file path
REM Example:     call :GET_PATH [path] [return variable name]
REM
REM *******************************************************************************************************
:GET_EXT 
set "%2=%~x1"
exit /b



REM *******************************************************************************************************
REM
REM HELP
REM Description: Provide help topic listing
REM *******************************************************************************************************
:HELP
if "%action%" equ "" goto HELP_MAIN
if "%action%" equ "LIST" goto HELP_LIST
if "%action%" equ "DELETE" goto HELP_DEL
if "%action%" equ "ADD" goto HELP_ADD
if "%action%" equ "BACKUP" goto HELP_BACKUP
if "%action%" equ "RESTORE" goto HELP_RESTORE
if "%action%" equ "CLEAN" goto HELP_CLEAN
if "%action%" equ "REFRESH" goto HELP_REFRESH
if "%action%" equ "PATHMGR" goto HELP_PATHMGR
if "%action%" equ "ENVIRONMENT" goto HELP_ENVIRONMENT

:HELP_MAIN
echo 	%filename% Version %version%
echo		Tool to manage the PATH environment variable. 
echo		Provides tools to add to , delete from, clean up,
echo		backup and restore the PATH environment variable 
echo		either on a session basis or make permanent changes
echo		in the registry. To get help type "pathmgr /?".
echo		Whenever %filename% is run a temporary environment 
echo		variable pathmgr_PATH is created that holds the
echo		current path of the %filename% command file. The
echo		/path switch may be used to add the path of %filename% itself.
echo.
echo 	SYNTAX:		%fileName% [/?^|-?^|] [action][modifier][scope][path] 
echo.
echo    	[/?^|-?]		Display help, further help can be obtained on
echo				a topic by including an action switch.
echo				Example: pathmgr /? /List  will provide help on list action.
echo.
echo		[action]	The management action to perform. Perermited values are:-
echo				/list ^| /add ^| /del[ete] ^| /clean ^|/backup ^| /restore ^| /refresh ^| /path ^| /[env]ironment  
echo.
echo		[modifier]	Some actions may have additional options.  
echo					 /x: Expand environment variables.  
echo					 /r: Reverse order.
echo					 /v: validate a path(s) existence.
echo					/nv: no validation of path(s) existence.
echo					 /p: display all prompts
echo					 /y: hide prompts and choose yes
echo					 /n: hide prompts and choose no
echo					 /+: add current %filename% to PATH
echo					 /-: delete current %filename% from PATH
echo.
echo		[scope]		The scope within the registry to perform the action.
echo				Permited values are:-  /user ^| /system ^| /all.
echo				The default scope is user as there are fewer 
echo				policy restrictions. System and All scopes may require
echo				administrative privileges.
echo.
echo		[path]		Depending on context this will be either a folder or file path.
echo				If the path is the only argument then the default action is /add.
echo				Environment variables may be used in paths. The easiest way to do this
echo				is to enclose the variable name within question marks '?' rather than 
echo				using the percent sign '%', as an example ?systemroot?.
echo				This practice has the advantage of working either from the commandline or
echo				working within a batch. The alternative is to escape the variable as below.
echo				If used on the command line use:-  pathmgr /add ?systemroot?" or
echo				pathmgr /add "^%%systemroot^%%". If used within a batch use
echo				pathmgr /add   "?systemroot?" or pathmgr /add "%%%%systemroot%%%%".
echo				If these escaped paths are not used the path will be expanded prior to being
echo				interpreted so if you really want the path to be %systemroot% without escaping
echo				the path will be added as "c:\windows".
echo.
goto EXIT

:HELP_PATH
echo		[path]		Environment variables may be used in paths. The easiest way to do this
echo				is to enclose the variable name within question marks '?' rather than 
echo				using the percent sign '%', as an example ?systemroot?.
echo				This practice has the advantage of working either from the commandline or
echo				working within a batch. The alternative is to escape the variable as below.
echo				If used on the command line use:-  pathmgr /add ?systemroot?" or
echo				pathmgr /add "^%%systemroot^%%". If used within a batch use
echo				pathmgr /add   "?systemroot?" or pathmgr /add "%%%%systemroot%%%%".
echo				If these escaped paths are not used the path will be expanded prior to being
echo				interpreted so if you really want the path to be %systemroot% without escaping
echo				the path will be added as "c:\windows".
echo.
echo.
exit /b

:HELP_LIST
echo		LIST PATHS
echo		Displays a list of paths in user or system or both registry paths.
echo		according to the scope switch. 
echo.
echo		SYNTAX:		%fileName% ^/list [scope][modifiers]
echo.
echo		[scope]		^/user^|^/system^|^/all
echo.
echo		[modifiers]	/x expand all paths before display.
echo				/r display paths in reverse order.
echo				/v paths that are not valid are indicated with 'does not exist'.
goto EXIT

:HELP_ENVIRONMENT
echo		ENVIRONMENT
echo		Displays a list of the current path environment variable.
echo.
echo		SYNTAX:		%fileName% ^/env[ironment] [modifiers]
echo.
echo		[modifiers]	/r display paths in reverse order.
echo				/v paths that are not valid are indicated with 'does not exist'.
goto EXIT


:HELP_DEL
echo		DELETE PATH	 
echo		Deletes a given path from the PATH variable.
echo		will search both user and system path.
echo		By default will prompt if you want to make a permanaent
echo		change to the registry. For changes to system paths you 
echo		may require adminstrative privileges.
echo.
echo		SYNTAX:		%fileName% /del[ete] [modifiers] [path]
echo.
echo		[modifiers]	/x Expand all paths before display.
echo				/r Display paths in reverse order.
echo				/p Display prompt to make registry change.
echo				/y Do not ask, just make registry change (batch mode).   
echo				/n Do not make registry change (batch mode).   
echo				/v Delete any paths that are not valid (do not need to provide any path).
echo.
call :HELP_PATH
echo.
echo		For drag and drop operation it is recommended that you create a shortcut file
echo		and include the following target - pathmgr /delete /y. This will delete the dropped path
echo		from whatever path it is found in the registry without prompting. However, if the path is in
echo		the system path you will require administrator privileges. In Vista and Windows7 this means
echo		that the shortcut needs to be checked as 'run as administrator'.
echo.
goto EXIT

:HELP_ADD
echo		ADD PATH	 
echo		Will add a given path to the PATH variable.
echo		By default the user HKCU path variable is used.
echo		Add is the default action if no action argument is provided
echo		but a valid path is provided. This makes it possible to have
echo		drag and drop. If a file path is dropped only the folder path is added.
echo		By default Add will prompt if you want to make a permanaent
echo		change to the registry. This behaviour can be changed by applying
echo		the /y or /n modifiers. 
echo.
echo		SYNTAX:		%fileName% /add [scope] [modifiers] [path]
echo.
echo		[scope]		^/user^|^/system
echo.
echo		[modifiers]	 /v By default will validate a path before adding.
echo				/nv Do not validate any paths before adding them.
echo				 /p Display prompt to make registry change.
echo				 /y Do not ask, just make registry change (batch mode).   
echo				 /n Do not make registry change (batch mode).  
echo.
call :HELP_PATH
echo.
echo		For drag and drop operation it is recommended that you create a shortcut file
echo		and include the following target - pathmgr /add /y. This will add the dropped path to
echo		the user path in the registry without prompting. 
echo.
goto EXIT

:HELP_BACKUP
echo		BACKUP
echo		Backs up either user or system or both paths.
echo		By default will backup the user path only.
echo		Must use the /all scope switch to backup both user and system.
echo.
echo		SYNTAX:		%fileName% /backup [scope] [path]
echo.
echo		[scope]		^/user^|^/system^|^/all
echo.
call :HELP_PATH
echo				The path requires an output file path, a folder path will result in an error.
echo				Each path in the PATH variable is output on its own line in the output file.
echo				User paths are under heading 'USER PATH' and system paths are under the
echo				heading 'SYSTEM PATH'. 
goto EXIT

:HELP_RESTORE
echo		RESTORE
echo		Restores paths from a backup text file.
echo.
echo		SYNTAX:		%fileName% ^/restore [path]
echo.
call :HELP_PATH
echo				The path requires a valid backup file path.
echo				restore process is aborted if the first line in backup file is not
echo				either 'USER PATH' or 'SYSTEM_PATH'.
echo.				
goto EXIT

:HELP_CLEAN
echo		CLEAN
echo		Will clean up the PATH variable in the registry by removing 
echo		trailing and leading spaces, remove any enclosing double quotes,
echo		remove duplicate paths and if /v specified will remove invalid paths.
echo		The scope indicates wheteher user, system or both paths are cleaned. 
echo		By default will prompt if you want to make a permanaent
echo		change to the registry. Note that any changes made to the path variable 
echo		not made permanent in the registry will be lost. 
echo		If you do not have administrator privileges you will not be able
echo		to make changes to the system path. 
echo.
echo		SYNTAX:		%fileName% /clean [scope] [modifiers]
echo.
echo		[scope]		^/user^|^/system^|^/all
echo.
echo		[modifiers]	 /v Will also delete any invalid paths
echo				 /p Display prompt to make registry change.
echo				 /y Do not ask, just make registry change (batch mode).   
echo				 /n Do not make registry change (batch mode).   
echo.
goto EXIT

:HELP_REFRESH
echo		REFRESH
echo		Will refresh the current environment PATH variable with
echo		the user and system paths in the registry.Any changes
echo		made to the path variable made in the registry will be lost.  
echo.
echo		SYNTAX:		%fileName% /refresh 
goto EXIT

:HELP_PATHMGR
echo		PATH
echo		Will display the current path of %filename%.
echo		By using the modifier /+ or /- you can also add or delete
echo		the %filename% path from the PATH variable.
echo.
echo		SYNTAX:		%fileName% /path [scope] [modifiers]
echo.
echo		[scope]		^/user^|^/system^|/all
echo.
echo		[modifiers]	 /+: add current %filename% to PATH
echo				 /-: delete current %filename% from PATH
echo				 /p Display prompt to make registry change.
echo				 /y Do not ask, just make registry change (batch mode).   
echo				 /n Do not make registry change (batch mode).   
echo.
 
goto EXIT



:EXIT
echo. 
if "%help%" equ "false" ( echo %fileName% %action% action complete.
 			  echo. )
if defined newPath  (endlocal &  set "path=%newPath%" & set "pathmgr_Path=%pathmgrPath%") else (endlocal & set "pathmgr_Path=%pathmgrPath%")

@echo on


