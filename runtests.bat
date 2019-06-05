@ECHO OFF
SET DBGDIR=%LOCALAPPDATA%\DBG\UI\Fast.20190603.4\amd64
SET BINDIR=%~dp0x64

IF NOT EXIST "%DBGDIR%\dbgeng.dll" (
  ECHO File 'dbgeng.dll' not found in %DBGDIR%. Please update %0
  EXIT /B 1
)

IF NOT EXIST "%BINDIR%\testcons.exe" (
  ECHO Debugger extension 'dbgext.dll' not found. Please build first.
  EXIT /B 1
)

ROBOCOPY "%BINDIR%" "%DBGDIR%" dbgext.dll dbgext.pdb testcons.exe testcons.pdb > NUL

IF /I "%1" == "dbg" (
  windbgx "%DBGDIR%\testcons.exe"
) ELSE (
  "%DBGDIR%\testcons.exe"
)
