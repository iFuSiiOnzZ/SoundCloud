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

:: Compiler flags
SET CommonCompilerFlags= /nologo /Od /Z7 /EHsc /Fd%PDBFiles% /Fo%IntermediatePath% /Fe%ExePath%
SET Defines=-DX64

:: Compile
call cl %CommonCompilerFlags% %Defines% build.cpp