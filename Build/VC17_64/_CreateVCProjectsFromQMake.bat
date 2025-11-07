rem echo off
:: Compiler
IF EXIST "c:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
	call "c:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
) ELSE (
	IF EXIST "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsx86_amd64.bat" (
		call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsx86_amd64.bat"
	) ELSE (
		call "c:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64
	)
)


set COMPILER_EXT=VC17_64
set QMAKESPEC=%QTDIR%\mkspecs\win32-msvc
set path=%path%;%QTDIR%\bin

echo Generating %COMPILER_EXT% projects...

cd %~dp0\..\..\Build\QMake
%QTDIR%\bin\qmake -recursive -tp vc


cd %~dp0\..\..
call %ACFCONFIGDIR%\QMake\CopyVCProjToSubdir.js %COMPILER_EXT%

cd %~dp0\
