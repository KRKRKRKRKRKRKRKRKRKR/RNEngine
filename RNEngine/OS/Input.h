#pragma once
#include <array>							//std::array
#include "Math/Vector.h"

class Window;

//キーボード・マウスの状態(毎フレーム1回Updateで取り込む)
class Input
{
public:
	void Update(Window& window);			//今の状態を取り込み、前の状態を残す

	bool IsDown(int key) const;				//押されている間ずっと(マウスもVK_LBUTTON等で可)
	bool IsPressed(int key) const;			//押された瞬間(1フレームだけ)
	bool IsReleased(int key) const;			//離された瞬間(1フレームだけ)

	Vector2 GetMousePosition() const { return m_mouse; }		//ウィンドウ内のピクセル座標(左上0,0)
	Vector2 GetMouseDelta() const { return m_mouseDelta; }		//前フレームからの移動量
	float GetWheel() const { return m_wheel; }					//このフレームの回転量(1=1目盛り、奥が+)

private:
	std::array<unsigned char, 256> m_now = {};	//今のフレーム(仮想キーコードが添字)
	std::array<unsigned char, 256> m_prev = {};	//前のフレーム
	Vector2 m_mouse = { 0.0f, 0.0f };
	Vector2 m_mouseDelta = { 0.0f, 0.0f };
	float m_wheel = 0.0f;
};
