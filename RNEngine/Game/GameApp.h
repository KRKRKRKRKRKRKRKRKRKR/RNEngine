#pragma once
#include "Engine/Game.h"

class Model;
struct Sprite;
class Spinner;

//実際のゲームの中身
class GameApp : public Game
{
public:
	void Init(Engine& engine) override;
	void Update(Engine& engine, float dt) override;
	void Draw(Engine& engine) override;
	void DrawUI(Engine& engine) override;

private:
	int texture0 = -1;						//画像の番号
	int texture1 = -1;


	Sprite* m_sprite = nullptr;
};
