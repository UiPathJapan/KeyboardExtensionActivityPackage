@ECHO OFF
REM ARG1 - DIRECTORY where NuGet packages are placed
IF "%1" EQU "" (
  ECHO ERROR: BAD COMMAND LINE SYNTAX! 1>&2
  EXIT /B 1
)
ECHO 2.2.0 > NextVersion.tmp
FOR /F %%a IN ('CMD /C DIR %1\*.nupkg /O:D /B') DO (
  FOR /F "delims=. tokens=4,5,6" %%x IN ('CMD /C ECHO %%a') DO (
    FOR /F %%b IN ('CMD /C SET /A %%z+1') DO (
      ECHO %%x.%%y.%%b > NextVersion.tmp
    )
  )
)
TYPE NextVersion.tmp
DEL NextVersion.tmp
EXIT /B 0
