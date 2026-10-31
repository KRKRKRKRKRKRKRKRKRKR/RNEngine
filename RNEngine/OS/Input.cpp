#include "OS/Input.h"
#include "OS/Window.h"

static bool Down(unsigned char state)
{
	return (state & 0x80) != 0;				//最上位ビット=押されている
}

void Input::Update(Window& window)
{
	m_prev = m_now;							//前のフレームとして残す
	GetKeyboardState(m_now.data());			//256キー分をまとめて取得

	POINT p;
	GetCursorPos(&p);						//画面全体での位置
	ScreenToClient(window.GetHwnd(), &p);	//ウィンドウ内の位置へ
	Vector2 mouse = { (float)p.x, (float)p.y };
	m_mouseDelta = { mouse.x - m_mouse.x, mouse.y - m_mouse.y };	//前フレームとの差
	m_mouse = mouse;

	m_wheel = (float)window.TakeWheel() / WHEEL_DELTA;	//120→1目盛り
}

bool Input::IsDown(int key) const
{
	return Down(m_now[key]);
}

bool Input::IsPressed(int key) const
{
	return Down(m_now[key]) && !Down(m_prev[key]);	//今押されていて、前は押されていない
}

bool Input::IsReleased(int key) const
{
	return !Down(m_now[key]) && Down(m_prev[key]);
}
