#pragma once
#include "Engine/GameObject.h"

//回り続ける球(GameObjectの動作確認用)
class Spinner : public GameObject
{
public:
	Spinner(int texture, float x, float y);				//画像と出現位置
	void Init() override;
	void Update(float dt) override;

private:
	int m_texture = -1;
	Model* m_model = nullptr;							//借りているだけ(GameObjectが自動で削除)
	Model* m_moon = nullptr;							//周りを回る小さい球(親子関係の確認)
};
