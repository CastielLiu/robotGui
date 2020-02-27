@set pwd=%cd%   

@echo %pwd% 

@set disk=%~d0 

@call F:\Qt\Qt5.9.8\5.9.8\mingw53_32\bin\qtenv2.bat

@%disk%   
@cd %pwd% 

@echo %cd% 

windeployqt videoTransmission.exe
pause


