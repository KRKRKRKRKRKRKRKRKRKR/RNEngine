//ウィンドウの作成とメッセージ処理を行うクラス

#include "OS/Window.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);	//ImGui側の入力処理

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
		nullptr, nullptr, hInstance, this);	//thisをWM_NCCREATEで受け取る
}

//表示
void Window::Show(int nCmdShow)
{
	ShowWindow(m_hwnd, nCmdShow);
}

//中身の大きさを指定して変える
void Window::SetClientSize(UINT width, UINT height)
{
	if (IsZoomed(m_hwnd)) ShowWindow(m_hwnd, SW_RESTORE);	//最大化中は大きさを変えられないので戻す

	RECT rect = { 0, 0, (LONG)width, (LONG)height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);	//枠分を加算(Initと同じ計算)
	SetWindowPos(m_hwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOMOVE | SWP_NOZORDER);			//位置と重なり順はそのまま。WM_SIZEが届く
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

int Window::TakeWheel()
{
	int wheel = m_wheel;
	m_wheel = 0;							//次のフレームは0から
	return wheel;
}

bool Window::TakeResized()
{
	bool resized = m_resized;
	m_resized = false;
	return resized;
}

//メッセージ処理関数
LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp))	//ImGuiが処理したら終了
	{
		return true;
	}

	Window* self = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);	//staticなので保存したthisを取り出す

	switch (msg)
	{
	case WM_NCCREATE:						//最初のメッセージ
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lp)->lpCreateParams);	//thisを保存
		break;								//既定処理も必要
	case WM_MOUSEWHEEL:
		if (self) self->m_wheel += GET_WHEEL_DELTA_WPARAM(wp);	//奥=+、手前=-
		return 0;
	case WM_SIZE:							//サイズ変更(最小化も含む)
		if (self)
		{
			self->m_width = LOWORD(lp);		//中身(枠なし)の幅
			self->m_height = HIWORD(lp);
			self->m_resized = true;			//作り直しはEngineがループ内で行う
		}
		return 0;
	case WM_DESTROY:						//ウィンドウ破棄
		PostQuitMessage(0);					//WM_QUITを送る
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);	//既定処理
}