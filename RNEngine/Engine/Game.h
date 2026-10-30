#pragma once

class Engine;

//ゲームの基底クラス(中身は派生クラスが書く)
class Game
{
public:
	virtual ~Game() = default;

	virtual void Init(Engine& engine) {}				//最初に1回:物を置く・画像を読む
	virtual void Update(Engine& engine, float dt) {}	//毎フレーム:動かす(dt=経過秒)
	virtual void Draw(Engine& engine) {}				//毎フレーム:描画命令を出す
	virtual void DrawUI(Engine& engine) {}				//毎フレーム:ImGuiのウィンドウ
};
