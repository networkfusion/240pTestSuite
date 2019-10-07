@REM Path to the Huc compiler, The suite uses the one at: https://github.com/uli/huc
@set CYGWIN="nodosfilewarning"
@set HUC_HOME=C:\Devel\PCE\huc

@path=%path%;%HUC_HOME%\bin
@set PCE_INCLUDE=%HUC_HOME%/include/pce

CALL makeclean.bat

@REM Compile, assemble, and link
@REM -fno-recursive makes globals fail
huc -DSCDROM -DCDROM -s -t -O2 -msmall -scd -overlay loader.c font.c video.c  || goto :error
pceas -DSCDROM -DCDROM -s -msmall -scd -overlay loader.s  || goto :error
huc -DSCDROM -DCDROM -s -t -O2 -msmall -scd -overlay 240pSuite.c font.c video.c tests.c patterns.c tools.c help.c  || goto :error
pceas -DSCDROM -DCDROM -s -msmall -scd -overlay 240pSuite.s  || goto :error
isolink 240pSuite.iso loader.ovl 240pSuite.ovl adpcm.vox sweep.vox  || goto :error

:; exit 0
exit /b 0

:error
exit /b %errorlevel%