@echo off
setlocal enabledelayedexpansion
set product_version=""
set file_version=""
set tmp=""
set data=""
set err=0
set proc=0
:: setlocal
set GIT_PATH=D:\Program Files\Git\bin
set GIT_USR_PATH=D:\Program Files\Git\usr\bin
set PATH=%PATH%;%GIT_USR_PATH%

for /f %%i in ('git rev-list HEAD ^| wc -l') do (set file_version=%%i) 
echo %file_version%

for /f "delims= " %%i in ('git.exe rev-list HEAD --timestamp --max-count=1') do (set product_version=%%i) 
echo %product_version%

:: @exit /b
if exist win32\revision.h ( Del win32\revision.h /q)
if exist revision.h ( Del revision.h /q)

set /a str_len=0
set /a ver_err=0
call:strlen %product_version% str_len

if %str_len% GTR 0 ( set /a ver_err=0)else ( set /a ver_err=1)

for /f "delims=" %%i in (..\version\revision-template.h) do (
	set /a proc=0
	set data=%%i
	if !proc! EQU 0 ( 
		echo %%i | findstr /c:FILE_REVISION && set /a err=1 || set /a err=0
		if !err! EQU 1 ( 
			set /a proc=1
			set tmp=!data:$WCREV$=%file_version%!&echo !tmp! >> revision.h
		)
	)
	if !proc! EQU 0 ( 
		echo %%i | findstr /c:PRODUCT_REVISION && set /a err=1 || set /a err=0
		if !err! EQU 1 ( 
			set /a proc=1
			set tmp=!data:$WCREV$=%product_version%!&echo !tmp! >> revision.h
		)
	)
	if !proc! EQU 0 ( 
		echo %%i | findstr /c:$VER_ERROR$ && set /a err=1 || set /a err=0
		if !err! EQU 1 ( 
			set /a proc=1
			set tmp=!data:$VER_ERROR$=%ver_err%!&echo !tmp! >> revision.h
		)
	)
	if !proc! EQU 0 ( echo !data! >> revision.h)
)

:: endlocal
pause
goto:eof

:strlen <stringVarName> [retvar] 
:: 思路： 二分回溯联合查表法
:: 说明： 所求字符串大小范围 0K ~ 8K；
::    stringVarName ---- 存放字符串的变量名
::    retvar      ---- 接收字符长度的变量名
setlocal enabledelayedexpansion
:: set "$=!%1!#"
set "$=%1#"
set N=&for %%a in (8 4 2 1)do if !$:~%%a!. NEQ . set/aN+=%%a&set $=!$:~%%a!
set $=!$!fedcba9876543210&set/aN+=0x!$:~16,1!
endlocal&If %2. neq . (set/a%2=%N%)else echo %N%
goto:eof