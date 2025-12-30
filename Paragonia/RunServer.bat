@echo off
:: 1. 언리얼 에디터 실행 파일 경로 (버전/설치 위치에 따라 다름)
set UE5Editor="D:\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"

:: 2. 내 프로젝트 파일 경로 (따옴표 필수)
set MyProject="D:\Unreal_Projects\Chap4_TeamPro\Paragonia\Paragonia.uproject"

:: 3. 실행 (맵 이름, -server 옵션, -log 옵션 필수)
set ServerMap="/Game/Paragonia/Maps/Lobby"
%UE5Editor% %MyProject% %ServerMap% -server -log -nosteam

pause