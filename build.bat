@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars32.bat"

rem Duong dan TUONG DOI theo vi tri build.bat (%~dp0 = ...\edictbudget\).
rem Truoc day ghi cung "...\dichnguoc\..." -> thu muc bi doi ten thanh
rem "dichnguoc - Copy (2)" la build gay loi C1083 khong tim thay ISmmPlugin.h.
set HL2SDK=%~dp0..\hl2sdk-l4d2
set MMSOURCE=%~dp0..\metamod-source-1.12.0.1225

set INCLUDES=/I "%HL2SDK%\public" /I "%HL2SDK%\public\engine" /I "%HL2SDK%\public\tier0" /I "%HL2SDK%\public\tier1" /I "%HL2SDK%\public\vstdlib" /I "%HL2SDK%\public\mathlib" /I "%HL2SDK%\public\game\server" /I "%MMSOURCE%\core" /I "%MMSOURCE%\core\sourcehook"
set DEFINES=/D "WIN32" /D "_WINDOWS" /D "COMPILER_MSVC" /D "COMPILER_MSVC32" /D "SE_LEFT4DEAD2=16" /D "SOURCE_ENGINE=16"

rem ###########################################################################
rem  /O2 va ten dau ra edictbudget_mm.dll: DUNG DOI.
rem  08/08 phat hien build.bat CHUA TUNG la script tao ra ban dang chay:
rem    - no ghi /Od  -> code 176.128, file 231.424
rem    - ban proven  -> code 155.648, file 209.408
rem    - build thu lai voi /O2 -> code 155.648, file 209.408  (TRUNG KHIT)
rem  Va no ghi /OUT:edictbudget.dll trong khi VDF nap
rem  addons/edictbudget/bin/edictbudget_mm.dll -> hai ten khac nhau, de deploy nham.
rem ###########################################################################
cl.exe /MT /O2 /Zi /LD /EHsc %DEFINES% %INCLUDES% sample_mm.cpp "%HL2SDK%\lib\public\tier0.lib" "%HL2SDK%\lib\public\tier1.lib" "%HL2SDK%\lib\public\vstdlib.lib" /link /OUT:edictbudget_mm.dll /DEBUG /OPT:REF /OPT:ICF /MAP
