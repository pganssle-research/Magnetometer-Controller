:: Runs ncgen and ncdump tests.
:: when called from commandline exit closes the command processor...
echo on
..\%1\ncgen -b -o c0.nc ..\..\..\ncgen\c0.cdl
..\%1\ncdump -n c1 c0.nc 2>&1 1>c1.cdl
..\%1\ncgen -b c1.cdl
..\%1\ncdump c1.nc 2>&1 1>c2.cdl
fc c1.cdl c2.cdl | find /i "FC: no differences encountered" > nul
if errorlevel==1 goto ERR_LABEL
..\%1\ncgen -b ..\..\..\ncdump\test0.cdl
..\%1\ncdump -n test1 test0.nc 2>&1 1>test1.cdl
..\%1\ncgen -b test1.cdl
..\%1\ncdump test1.nc 2>&1 1>test2.cdl
fc test1.cdl test2.cdl | find /i "FC: no differences encountered" > nul
if errorlevel==1 goto ERR_LABEL
exit \B 0
:GOTO:EOF

:ERR_LABEL
echo ************** ERROR - Comparison Not Correct! **********************
exit \B 1

