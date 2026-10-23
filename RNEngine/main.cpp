#include "Application/Application.h"

//ここから始まる
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application app;
	app.Init(hInstance, nCmdShow);			//初期化
	app.Run();								//ループ
	return 0;								//終了(appのデストラクタで破棄)
}