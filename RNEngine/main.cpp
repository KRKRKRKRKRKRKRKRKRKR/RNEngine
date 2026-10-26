#include "Application/Application.h"
#include <DirectXTex.h>
//ここから始まる
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	CheckHR(CoInitializeEx(nullptr, COINIT_MULTITHREADED));	//WICに必要
	{
		Application app;
		app.Init(hInstance, nCmdShow);		//初期化
		app.Run();							//ループ
	}										//appをここで破棄
	CoUninitialize();
	return 0;
}