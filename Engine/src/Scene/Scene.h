#ifndef AERO3D_SCENE_SCENE_H_
#define AERO3D_SCENE_SCENE_H_

#include <vector>
#include <memory>

#include "Scene/Actor.h"

namespace aero3d {

class Scene 
{
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
