@echo off
REM run from the MSVC Developer Command Prompt or similar to:
REM C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat build.bat

MSBuild.exe AriaCoda.sln -property:Configuration=Debug -maxCpuCount 
