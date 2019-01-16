if "%1"=="" goto:eof
if "%2"=="" goto:eof
set cur=%cd%
set bat_path=%~dp0
cd %bat_path%
echo %2revision.h

subwcrev.exe %1 revision-template.h %2revision.h
cd %cur%