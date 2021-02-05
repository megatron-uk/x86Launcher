ECHO OFF

REM Start the launcher
:launchstart
launcher.exe >OUT.TXT


REM On exit, run.bat will contain the startup for our selected game
call run.bat


REM Once exited from the game, return back to launcher
REM goto launchstart

