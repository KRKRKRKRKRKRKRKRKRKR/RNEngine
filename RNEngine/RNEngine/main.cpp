#include <windows.h>
#include <d3d12.h>							//Dx12本体
#include <dxgi1_6.h>						//Factory,SwapChain
#include <d3dcompiler.h>					//シェーダーのコンパイル
#include <wrl.h>							//ComPtr

#pragma comment(lib, "d3d12.lib")			//Dx12本体
#pragma comment(lib, "dxgi.lib")			//Factory,SwapChain
#pragma comment(lib, "d3dcompiler.lib")		//シェーダーのコンパイル

using Microsoft::WRL::ComPtr;

const UINT WINDOW_WIDTH = 1280;				//描画幅
const UINT WINDOW_HEIGHT = 720;				//描画高さ

HWND g_hwnd = nullptr;						//ウィンドウハンドル

inline void CheckHR(HRESULT hr)				//失敗なら停止
{
	if (FAILED(hr))
	{
		MessageBoxA(nullptr, "HRESULT failed", "Error", MB_OK);
		__debugbreak();
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)	//メッセージ処理
{
	switch (msg)
	{
	case WM_DESTROY:						//ウィンドウ破棄
		PostQuitMessage(0);					//WM_QUITを送る
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);	//既定処理
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//1-2 クラス登録
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;		//サイズ変更時に再描画
	wc.lpfnWndProc = WndProc;				//メッセージ処理関数
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	//矢印カーソル
	wc.lpszClassName = L"RNEngineWindow";
	RegisterClassEx(&wc);

	//1-3 サイズ計算
	RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);	//枠分を加算

	//1-4 作成・表示
	g_hwnd = CreateWindow(
		wc.lpszClassName,
		L"RNEngine",						//タイトル
		WS_OVERLAPPEDWINDOW,				//通常ウィンドウ
		CW_USEDEFAULT, CW_USEDEFAULT,		//位置はOS任せ
		rect.right - rect.left,				//幅(枠込み)
		rect.bottom - rect.top,				//高さ(枠込み)
		nullptr, nullptr, hInstance, nullptr);
	ShowWindow(g_hwnd, nCmdShow);

	//1-5 メッセージループ
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))	//メッセージあり
		{
			TranslateMessage(&msg);			//キー入力変換
			DispatchMessage(&msg);			//WndProcへ送る
		}
		else
		{
			//ここで毎フレーム描画(Phase3以降)
		}
	}

	return 0;
}
