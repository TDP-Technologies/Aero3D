#include "Scene/Scene.h"

#include <algorithm>

namespace aero3d {

Scene::Scene()
{

}

Scene::~Scene()
{

}

void Scene::Update(float deltaTime)
{
    for (auto gameObject : m_GameObjects)
	{
		gameObject->Update(deltaTime);
	}
}

void Scene::AddGameObject(GameObject* gameObject)
{
    m_GameObjects.emplace_back(gameObject);
}

void Scene::RemoveGameObject(GameObject* gameObject)
{
    auto iter = std::find(m_GameObjects.begin(), m_GameObjects.end(), gameObject);
	if (iter != m_GameObjects.end())
	{
		std::iter_swap(iter, m_GameObjects.end() - 1);
		m_GameObjects.pop_back();
	}
}

} // namespace aero3d
