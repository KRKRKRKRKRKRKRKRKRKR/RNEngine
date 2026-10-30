#include "OS/Input.h"
#include <windows.h>

static bool Down(unsigned char state)
{
	return (state & 0x80) != 0;				//最上位ビット=押されている
}

void Input::Update()
{
	m_prev = m_now;							//前のフレームとして残す
	GetKeyboardState(m_now.data());			//256キー分をまとめて取得
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
