@ECHO OFF
SETLOCAL

:: ============================================================================
:: COMPILAZIONE DI RAM-Runner 64 BIT
:: ============================================================================

:: 1. Rileva dinamicamente il percorso di installazione di Visual Studio 2022 o versioni successive
:: Se usi Enterprise o Professional, vswhere troverà comunque il percorso corretto automaticamente.
FOR /F "usebackq tokens=*" %%I IN (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) DO (
	SET "VS_PATH=%%I"
)
IF NOT EXIST "%VS_PATH%" (
	ECHO Errore: Installazione di Visual Studio non trovata.
	EXIT /b 1
)

:: 2. Configura l'ambiente per gli strumenti HostX64 (permette di usare compilatori sia x86 che x64)
:: Chiamiamo lo script nativo che popolerà il PATH con ml.exe, ml64.exe e cl.exe corretti.
CALL "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" amd64_x86 > NUL

:: ----------------------------------------------------------------------------
:: 3. COMPILAZIONE COMPONENTI 32 BIT (x86)
:: ----------------------------------------------------------------------------
ECHO Compilazione file Assembly a 32 bit...
FOR %%F IN (AdLibA ProcA SpeakerA TimerA VideoA) DO (
	"%VS_PATH%\VC\Tools\MSVC\%VCToolsVersion%\bin\HostX64\X86\ml.exe" /c /coff %%F.asm
)

ECHO Compilazione file C++ a 32 bit...
FOR %%F IN (AdLib Audio Console Core32 Proc RR Timer Video) DO (
	"%VS_PATH%\VC\Tools\MSVC\%VCToolsVersion%\bin\HostX64\X86\cl.exe" /c /std:c++20 /Zl /GS- /Od /arch:IA32 %%F.cpp
)

:: ----------------------------------------------------------------------------
:: 4. COMPILAZIONE COMPONENTI 64 BIT (x64)
:: ----------------------------------------------------------------------------
ECHO Compilazione file Assembly a 64 bit...
FOR %%F IN (Main64 Core64A) DO (
	"%VS_PATH%\VC\Tools\MSVC\%VCToolsVersion%\bin\HostX64\X64\ml64.exe" /c %%F.asm
)

ECHO Compilazione file C++ a 64 bit...
FOR %%F IN (Core64) DO (
	"%VS_PATH%\VC\Tools\MSVC\%VCToolsVersion%\bin\HostX64\X64\cl.exe" /c /std:c++20 /Zl /GS- /Od %%F.cpp
)

:: ----------------------------------------------------------------------------
:: EMETTE AVVISO FINALE
:: ----------------------------------------------------------------------------
ECHO Compilazione completata.
ENDLOCAL
PAUSE
