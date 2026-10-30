#include "Engine/Engine.h"
#include "Game/MyGame.h"

//ここから始まる
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	CheckHR(CoInitializeEx(nullptr, COINIT_MULTITHREADED));	//WICに必要
	{
		Engine engine;
		engine.Init(hInstance, nCmdShow, 1280, 720, L"RNEngine");	//窓・GPU・組み込みの形
		MyGame game;
		engine.Run(game);					//ゲームの準備→ループ
	}										//gameとengineをここで破棄(game→engineの順)
	CoUninitialize();
	return 0;
}
