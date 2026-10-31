#pragma once
#include "Common.h"

//OSのウィンドウを管理する
class Window
{
public:
	~Window();

	void Init(HINSTANCE hInstance, UINT width, UINT height, const wchar_t* title);
	void Show(int nCmdShow);
	void SetClientSize(UINT width, UINT height);	//中身(枠なし)の大きさを変える
	bool ProcessMessages();					//falseなら終了

	HWND GetHwnd() const { return m_hwnd; }
	UINT GetWidth() const { return m_width; }
	UINT GetHeight() const { return m_height; }
	int TakeWheel();						//溜まったホイール量を返して0に戻す(120=1目盛り)
	bool TakeResized();						//前回からサイズが変わったか(読むとfalseに戻る)
	bool IsMinimized() const { return m_width == 0 || m_height == 0; }	//最小化中は0x0

private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);	//メッセージ処理

	static constexpr const wchar_t* CLASS_NAME = L"RNEngineWindow";

	HINSTANCE m_hInstance = nullptr;
	HWND m_hwnd = nullptr;
	UINT m_width = 0;
	UINT m_height = 0;
	int m_wheel = 0;						//WM_MOUSEWHEELの合計
	bool m_resized = false;					//WM_SIZEが来たらtrue
};