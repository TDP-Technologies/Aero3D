#ifndef AERO3D_SCENE_ACTOR_H_
#define AERO3D_SCENE_ACTOR_H_

#include <vector>
#include <memory>
#include <type_traits>

namespace aero3d {

class Component;
class SceneComponent;
class Scene;

class Actor 
{
public:
    Actor();
    virtual ~Actor();

    virtual void Update(float deltaTime);

    void SetScene(Scene* scene);
    Scene* GetScene() const;

    void SetRootComponent(SceneComponent* component);
    SceneComponent* GetRootComponent() const;

    void AddComponent(std::unique_ptr<Component> component);

    template<typename T>
    T* GetComponent();

private:
    Scene* m_Scene = nullptr;
    SceneComponent* m_RootComponent = nullptr;
    std::vector<std::unique_ptr<Component>> m_Components;
};

template<typename T>
T* Actor::GetComponent() 
{
    for (auto& comp : m_Components) 
    {
        if (auto* casted = dynamic_cast<T*>(comp.get()))
            return casted;
    }
    return nullptr;
}

} // namespace aero3d

#endif // AERO3D_SCENE_ACTOR_H_
