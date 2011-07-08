@echo off
REM This script is used to automatically start the buildbot upon login to windows
start /b c:\Git\bin\sh.exe -c '^(buildbot start /c/bb^&^) ^&^& tail -f /c/bb/twistd.log'
