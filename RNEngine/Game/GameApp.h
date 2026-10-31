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

	Model* m_triangle0 = nullptr;			//借りているだけ(deleteしない)
	Model* m_triangle1 = nullptr;
	Model* m_sphere = nullptr;
	Model* m_beetle = nullptr;				//Beatall.fbxから読んだ形
	Sprite* m_sprite = nullptr;
	Spinner* m_spinner = nullptr;			//GameObject(Destroy後はnullptrに戻す)
};
