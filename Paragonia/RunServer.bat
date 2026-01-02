@echo off
:: PC의 엔진 경로 확인 필수!
set UE5Editor="C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"

:: PC의 프로젝트 경로 확인 필수!
set MyProject="C:\Users\Friend\Documents\Paragonia\Paragonia.uproject"

set ServerMap="/Game/Paragonia/Maps/Lobby"

:: -server: 데디케이티드 서버 모드
:: -log: 로그 창 띄우기
:: -game: 에디터가 아닌 게임 모드로 실행 (중요)
%UE5Editor% %MyProject% %ServerMap% -server -game -log -EpicApp=ParagoniaServer

pause