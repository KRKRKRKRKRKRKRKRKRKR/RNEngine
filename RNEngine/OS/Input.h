#pragma once
#include <array>							//std::array

//キーボードの状態(毎フレーム1回Updateで取り込む)
class Input
{
public:
	void Update();							//今の状態を取り込み、前の状態を残す

	bool IsDown(int key) const;				//押されている間ずっと
	bool IsPressed(int key) const;			//押された瞬間(1フレームだけ)
	bool IsReleased(int key) const;			//離された瞬間(1フレームだけ)

private:
	std::array<unsigned char, 256> m_now = {};	//今のフレーム(仮想キーコードが添字)
	std::array<unsigned char, 256> m_prev = {};	//前のフレーム
};
