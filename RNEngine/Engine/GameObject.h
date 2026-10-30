#pragma once
#include <vector>							//std::vector
#include "Math/Transform.h"
#include "Math/Vector.h"

class Engine;
class Model;
struct Sprite;

//ゲームに出てくる物の基底クラス(Engine::Spawnで出す)
class GameObject
{
public:
	virtual ~GameObject();								//自分が作ったModel/Spriteを削除
	virtual void Init() {}								//出現したとき1回
	virtual void Update(float dt) {}					//毎フレーム(dt=経過秒)

	void Destroy() { m_alive = false; }					//削除の予約(フレームの終わりに消える)
	bool IsAlive() const { return m_alive; }

	Transform transform;								//この物の位置

protected:
	Engine& GetEngine() { return *m_engine; }
	Model* CreateModel(int mesh, int texture);			//作ったModelを記録(破棄時に自動削除)
	Model* CreateTriangle(int texture);
	Model* CreateSphere(int texture);
	Sprite* CreateSprite(int texture, const Vector2& position, const Vector2& size);

private:
	friend class Engine;								//m_engineはSpawnが設定する
	Engine* m_engine = nullptr;
	bool m_alive = true;
	std::vector<Model*> m_models;						//自分が作ったModel
	std::vector<Sprite*> m_sprites;						//自分が作ったSprite
};
