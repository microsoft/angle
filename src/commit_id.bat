@echo off

REM check if git is installed
set _Git=(call where git)
If "%_Git%"=="%_Git:git=%" GOTO GitIsntInstalled

:GitIsInstalled

REM commit hash
(FOR /F "delims=" %%i IN ('call git rev-parse --short^=12 HEAD') DO set _Str=%%i) || (set _Str=badf00dbad00)
set _Str=#define ANGLE_COMMIT_HASH "%_Str%"
echo %_Str% > %3%

REM commit hash size
set _Str=#define ANGLE_COMMIT_HASH_SIZE 12
echo %_Str% >> %3%

REM commit date
(FOR /F "delims=" %%i IN ('call git show -s --format^="%%ci" HEAD') DO set _Str=%%i) || (set _Str=Unknown Date)
set _Str=#define ANGLE_COMMIT_DATE "%_Str%"
echo %_Str% >> %3%

exit

:GitIsntInstalled

REM commit hash
set _Str=#define ANGLE_COMMIT_HASH "invalid-hash"
echo %_Str% > %3%

REM commit hash size
set _Str=#define ANGLE_COMMIT_HASH_SIZE 12
echo %_Str% >> %3%

REM commit date
set _Str=#define ANGLE_COMMIT_DATE "invalid-date"
echo %_Str% >> %3%