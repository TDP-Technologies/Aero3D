#include "Scene/GameObject.h"

#include <algorithm>

#include "Core/Application.h"

namespace aero3d {

GameObject::GameObject(std::string objectName)
{
    name = objectName;

    Application::GetScene()->AddGameObject(this);
}

GameObject::~GameObject()
{
    Application::GetScene()->RemoveGameObject(this);

    while (!m_Components.empty())
	{
		delete m_Components.back();
	}
}

void GameObject::Update(float deltaTime)
{
    UpdateComponents(deltaTime);
    UpdateGameObject(deltaTime);
}

void GameObject::UpdateComponents(float deltaTime)
{
    for (Component* component : m_Components)
	{
		component->Update(deltaTime);
	}
}

void GameObject::AddComponent(Component* component)
{
    int myOrder = component->updateOrder;
	auto iter = m_Components.begin();
	for (; iter != m_Components.end(); ++iter)
	{
		if (myOrder < (*iter)->updateOrder)
		{
			break;
		}
	}

	m_Components.insert(iter, component);
}

void GameObject::RemoveComponent(Component* component)
{
    auto iter = std::find(m_Components.begin(), m_Components.end(), component);
	if (iter != m_Components.end())
	{
		m_Components.erase(iter);
	}
}

} // namespace aero3d
