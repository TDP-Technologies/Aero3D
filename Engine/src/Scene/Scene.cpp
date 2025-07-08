#include "Scene/Scene.h"

#include <algorithm>

namespace aero3d {

Scene::Scene()
{

}

Scene::~Scene()
{
    while (!m_GameObjects.empty())
	{
		delete m_GameObjects.back();
	}
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

void Scene::RemoveGameObject(std::string name)
{
    auto iter = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
        [&name](GameObject* obj) {
            return obj->name == name;
        });

    if (iter != m_GameObjects.end())
    {
        delete *iter;
        std::iter_swap(iter, m_GameObjects.end() - 1);
        m_GameObjects.pop_back();
    }
}

GameObject* Scene::GetGameObject(std::string name)
{
    auto iter = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
        [&name](GameObject* obj) {
            return obj->name == name;
        });

    if (iter != m_GameObjects.end())
        return *iter;

    return nullptr;
}

} // namespace aero3d
