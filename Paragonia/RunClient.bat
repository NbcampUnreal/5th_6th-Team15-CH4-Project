@echo off
set UE5Editor="D:\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
set MyProject="D:\Unreal_Projects\Chap4_TeamPro\Paragonia\Paragonia.uproject"

%UE5Editor% %MyProject% %ClientMap% -game -log ^
-EpicApp=ParagoniaClient ^
-LogOnline=VeryVerbose -LogOnlineSession=VeryVerbose -LogEOSSDK=VeryVerbose

pause
