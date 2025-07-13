#ifndef AERO3D_SCENE_SCENE_H_
#define AERO3D_SCENE_SCENE_H_

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

#include "Scene/Actor.h"

namespace aero3d {

class Scene 
{
public:
    template<typename T>
    static void RegisterActor();

    template<typename T>
    static void RegisterComponent();

    template<typename T>
    static std::unique_ptr<T> CreateActor();

    template<typename T>
    static std::unique_ptr<T> CreateComponent();

public:
    static std::unordered_map<std::size_t, std::function<std::unique_ptr<Actor>()>> s_ActorRegistry;
    static std::unordered_map<std::size_t, std::function<std::unique_ptr<Component>()>> s_ComponentRegistry;

public:
    void AddActor(std::unique_ptr<Actor> actor);
    void Update(float deltaTime);

    std::vector<std::unique_ptr<Actor>>& GetActors() { return m_Actors; };

    template<typename T>
    T* GetFirstComponentOfType();

    template<typename T>
    Actor* GetFirstActorWithComponent();

    template<typename T>
    std::vector<T*> GetAllComponentsOfType();

    template<typename... Ts>
    std::vector<Actor*> GetAllActorsWithComponents();

private:
    std::vector<std::unique_ptr<Actor>> m_Actors;

};

template<typename T>
void Scene::RegisterActor() 
{
    s_ActorRegistry[typeid(T).hash_code()] = []() 
    {
        return std::make_unique<T>();
    };
}

template<typename T>
void Scene::RegisterComponent() 
{
    s_ComponentRegistry[typeid(T).hash_code()] = []() 
    {
        return std::make_unique<T>();
    };
}

template<typename T>
std::unique_ptr<T> Scene::CreateActor() 
{
    auto it = s_ActorRegistry.find(typeid(T).hash_code());
    if (it != s_ActorRegistry.end()) 
    {
        return std::unique_ptr<T>(static_cast<T*>(it->second().release()));
    }
    return nullptr;
}

template<typename T>
std::unique_ptr<T> Scene::CreateComponent() 
{
    auto it = s_ComponentRegistry.find(typeid(T).hash_code());
    if (it != s_ComponentRegistry.end()) 
    {
        return std::unique_ptr<T>(static_cast<T*>(it->second().release()));
    }
    return nullptr;
}

template<typename T>
T* Scene::GetFirstComponentOfType() 
{
    for (auto& actor : m_Actors) 
    {
        if (T* comp = actor->GetComponent<T>()) 
        {
            return comp;
        }
    }
    return nullptr;
}

template<typename T>
Actor* Scene::GetFirstActorWithComponent() 
{
    for (auto& actor : m_Actors)
    {
        if (actor->GetComponent<T>()) 
        {
            return actor.get();
        }
    }
    return nullptr;
}

template<typename T>
std::vector<T*> Scene::GetAllComponentsOfType() 
{
    std::vector<T*> result;
    for (auto& actor : m_Actors)
    {
        if (T* comp = actor->GetComponent<T>()) 
        {
            result.push_back(comp);
        }
    }
    return result;
}

template<typename... Ts>
std::vector<Actor*> Scene::GetAllActorsWithComponents() 
{
    std::vector<Actor*> result;

    for (auto& actor : m_Actors) 
    {
        if ((actor->GetComponent<Ts>() && ...)) 
        {
            result.push_back(actor.get());
        }
    }

    return result;
}

} // namespace aero3d

#endif // AERO3D_SCENE_SCENE_H_
