@ECHO OFF

SET ROOT_PATH=%cd%
SET DIST_WIN=dist/x86_64-win64-gvs
SET CPP_VERS=c++2a
SET COMPILER=zig c++

ECHO Building %DIST_WIN% . . .

rmdir /S /Q %DIST_WIN%
mkdir %DIST_WIN%
%COMPILER% -g main.cpp -o %DIST_WIN%/gvs.exe -static -std=%CPP_VERS% -m64 -Ofast -Werror -Wall -Wextra -pedantic

ECHO Finished building.
ECHO Testing build . . .

@ECHO ON

%ROOT_PATH%/%DIST_WIN%/gvs.exe "syntax.gvs"

PAUSE