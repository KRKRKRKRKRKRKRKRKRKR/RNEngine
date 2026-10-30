#include "Engine/GameObject.h"
#include "Engine/Engine.h"

GameObject::~GameObject()
{
	if (!m_engine) return;								//Spawnされていない(何も作っていない)
	for (Model* m : m_models) m_engine->Destroy(m);		//Engineの一覧から外す
	for (Sprite* s : m_sprites) m_engine->Destroy(s);
}

Model* GameObject::CreateModel(int mesh, int texture)
{
	Model* m = m_engine->CreateModel(mesh, texture);
	m->transform.parent = &transform;					//自分に付いてくる
	m_models.push_back(m);								//覚えておく
	return m;
}

Model* GameObject::CreateTriangle(int texture)
{
	Model* m = m_engine->CreateTriangle(texture);
	m->transform.parent = &transform;					//自分に付いてくる
	m_models.push_back(m);								//覚えておく
	return m;
}

Model* GameObject::CreateSphere(int texture)
{
	Model* m = m_engine->CreateSphere(texture);
	m->transform.parent = &transform;
	m_models.push_back(m);
	return m;
}

Sprite* GameObject::CreateSprite(int texture, const Vector2& position, const Vector2& size)
{
	Sprite* s = m_engine->CreateSprite(texture, position, size);
	m_sprites.push_back(s);
	return s;
}
