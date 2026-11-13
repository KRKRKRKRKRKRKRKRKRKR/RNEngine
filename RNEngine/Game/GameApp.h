#pragma once
#include <vector>							//ボール・軌跡の一覧
#include <random>							//乱数
#include "Engine/Game.h"
#include "Math/Vector.h"

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
	//軌跡の1個(残した場所と、残してからの経過秒)
	struct TrailPoint
	{
		Vector2 position;
		float age = 0.0f;
	};

	//ボール1個(動き・色・軌跡はボールごとに別)
	struct Ball
	{
		Sprite* sprite = nullptr;			//位置・大きさ・色(借りているだけ)
		float speedX = 0.0f;				//右へ進む速さ(px/秒)
		float speedY = 0.0f;				//縦の速さ(px/秒、+は下)
		float gravity = 0.0f;				//重力(px/秒^2)
		float bounce = 0.0f;				//跳ね返ったあとに残る速さの割合
		float trailTimer = 0.0f;			//次の軌跡までの経過秒
		std::vector<TrailPoint> trail;		//軌跡(古い順)
	};

	void ResetBall(Engine& engine, Ball& ball);				//左上から、新しい乱数の動きで落とし直す
	void UpdateBall(Engine& engine, Ball& ball, float dt);	//動かして、軌跡を残す
	void DrawTrail(Engine& engine);							//全ボールの軌跡(板ポリ)を描く
	float RandomRange(float min, float max);				//min〜maxの乱数

	int texture0 = -1;						//画像の番号
	int texture1 = -1;
	int textureWhite = -1;					//白1x1(色はSpriteのcolorで付ける)

	Sprite* m_sprite = nullptr;

	//ソリティア風の演出(ピクセル座標・y下向きが+)
	static constexpr int BALL_COUNT = 3;
	std::vector<Ball> m_balls;
	float m_trailInterval = 0.03f;			//軌跡を残す間隔(秒)
	float m_trailLifeTime = 2.0f;			//軌跡が消えるまでの秒(寿命)
	std::mt19937 m_random{ std::random_device{}() };	//乱数の元(起動のたびに違う)
};
