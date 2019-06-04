SET DBGDIR=%LOCALAPPDATA%\DBG\UI\Fast.20190603.4\amd64
SET BINDIR=%~dp0x64

COPY /B /Y "%BINDIR%\dbgext.dll" "%DBGDIR%"
COPY /B /Y "%BINDIR%\dbgext.pdb" "%DBGDIR%"
COPY /B /Y "%BINDIR%\testcons.exe" "%DBGDIR%"
COPY /B /Y "%BINDIR%\testcons.pdb" "%DBGDIR%"

IF /I "%1" == "dbg" (
  windbgx "%DBGDIR%\testcons.exe"
) ELSE (
  "%DBGDIR%\testcons.exe"
)
