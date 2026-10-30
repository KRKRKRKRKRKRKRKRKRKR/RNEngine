#include "Engine/Engine.h"
#include "Game/GameApp.h"

//ここから始まる
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Log::Init("log.txt");									//最初に開く
	Log::Info("Start");
	CheckHR(CoInitializeEx(nullptr, COINIT_MULTITHREADED));	//WICに必要
	{
		Engine engine;
		engine.Init(hInstance, nCmdShow, 1280, 720, L"RNEngine");	//窓・GPU・組み込みの形
		GameApp game;
		engine.Run(game);					//ゲームの準備→ループ
	}										//gameとengineをここで破棄(game→engineの順)
	CoUninitialize();
	Log::Info("End");
	Log::Shutdown();										//最後に閉じる
	return 0;
}
