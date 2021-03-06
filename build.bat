@ECHO OFF

:: Create folders if not exist
IF NOT EXIST ..\Bin MKDIR ..\Bin
IF NOT EXIST ..\Bin\App MKDIR ..\Bin\App
IF NOT EXIST ..\Bin\Intermediate MKDIR ..\Bin\Intermediate

:: Set paths
SET ExeName=SoundCloud
SET ExePath=..\Bin\App\%ExeName%

SET PDBFiles=..\Bin\App\
SET IntermediatePath=..\Bin\Intermediate\

:: Delete files from paths
DEL /Q /S %IntermediatePath%*.* 1>NUL
DEL /Q %PDBFiles%*.pdb 2>NUL

DEL /Q %PDBFiles%*.Exe 2>NUL
DEL /Q %PDBFiles%*.ilk 2>NUL

:: Load compiler
WHERE cl >nul 2>nul
IF %ERRORLEVEL% NEQ 0 (
    CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
)

:: Resource files
SET ResourceName=%IntermediatePath%resource.res
call rc /nologo /fo %ResourceName% res\resource.rc

:: Compiler flags
SET CommonCompilerFlags= /nologo /O2 /MT /Z7 /EHsc  /Fd%PDBFiles% /Fo%IntermediatePath% /Fe%ExePath%
SET CommonLinkerFlags= -incremental:no -opt:ref User32.lib Shell32.lib Ole32.lib Gdi32.lib
SET Defines=-DX64

:: Compile
call cl %CommonCompilerFlags% %Defines% build.cpp /link %CommonLinkerFlags% %ResourceName%

:: Embeb the manifest
call mt.exe -nologo -manifest "%ExePath%.exe.manifest" -outputresource:"%ExePath%.exe;1"
call del %ExePath%.exe.manifest