#include "Scene/Actor.h"
#include "Scene/Components.h"

namespace aero3d {

Actor::Actor()
{
}

Actor::~Actor()
{
}

void Actor::Update(float deltaTime)
{
    for (auto& comp : m_Components)
    {
        comp->Update(deltaTime);
    }
}

void Actor::SetScene(Scene* scene)
{
    m_Scene = scene;
}

Scene* Actor::GetScene() const
{
    return m_Scene;
}

void Actor::SetRootComponent(SceneComponent* component)
{
    m_RootComponent = component;
}

SceneComponent* Actor::GetRootComponent() const
{
    return m_RootComponent;
}

void Actor::AddComponent(std::unique_ptr<Component> component)
{
    component->SetOwner(this);
    m_Components.push_back(std::move(component));
}

} // namespace aero3d
