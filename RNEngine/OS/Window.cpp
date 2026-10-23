//ウィンドウの作成とメッセージ処理を行うクラス

#include "OS/Window.h"

//ウィンドウ破棄時にクラス登録を解除
Window::~Window()
{
	if (m_hInstance)
	{
		UnregisterClass(CLASS_NAME, m_hInstance);	//登録解除
	}
}

//初期化
void Window::Init(HINSTANCE hInstance, UINT width, UINT height, const wchar_t* title)
{
	m_hInstance = hInstance;
	m_width = width;
	m_height = height;

	//クラス登録
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;		//サイズ変更時に再描画
	wc.lpfnWndProc = WndProc;				//メッセージ処理関数
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	//矢印カーソル
	wc.lpszClassName = CLASS_NAME;
	RegisterClassEx(&wc);

	//サイズ計算
	RECT rect = { 0, 0, (LONG)width, (LONG)height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);	//枠分を加算

	//作成
	m_hwnd = CreateWindow(
		CLASS_NAME,
		title,
		WS_OVERLAPPEDWINDOW,				//通常ウィンドウ
		CW_USEDEFAULT, CW_USEDEFAULT,		//位置はOS任せ
		rect.right - rect.left,				//幅(枠込み)
		rect.bottom - rect.top,				//高さ(枠込み)
		nullptr, nullptr, hInstance, nullptr);
}

//表示
void Window::Show(int nCmdShow)
{
	ShowWindow(m_hwnd, nCmdShow);
}

//メッセージ処理
bool Window::ProcessMessages()
{
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))	//溜まった分を全部処理
	{
		if (msg.message == WM_QUIT)
		{
			return false;					//終了
		}
		TranslateMessage(&msg);				//キー入力変換
		DispatchMessage(&msg);				//WndProcへ送る
	}
	return true;							//続行
}

//メッセージ処理関数
LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:						//ウィンドウ破棄
		PostQuitMessage(0);					//WM_QUITを送る
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);	//既定処理
}