@ECHO OFF

SET ROOT_PATH=%cd%
SET DIST_WIN=dist/x86_64-win64-gvs

%ROOT_PATH%/%DIST_WIN%/gvs.exe "syntax.gvs"

PAUSE