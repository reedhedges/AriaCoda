@echo off
REM run with the MSVC Developer Command Prompt like this:
REM C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevComd.bat build.bat

MSBuild.exe AriaCoda.sln -property:Configuration=Debug -maxCpuCount 
