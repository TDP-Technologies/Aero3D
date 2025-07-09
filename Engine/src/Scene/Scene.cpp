#include "Scene/Scene.h"
#include "Scene/Actor.h"
#include "Scene/Components.h"

namespace aero3d {

void Scene::AddActor(std::unique_ptr<Actor> actor) 
{
    actor->SetScene(this);
    m_Actors.emplace_back(std::move(actor));
}

void Scene::Update(float deltaTime) 
{
    for (auto& actor : m_Actors) 
    {
        actor->Update(deltaTime);

        if (auto* script = actor->GetComponent<ScriptComponent>()) 
        {
            script->OnUpdate(deltaTime);
        }
    }
}

void Scene::Render() 
{
    for (auto& actor : m_Actors) 
    {
        if (auto* mesh = actor->GetComponent<MeshComponent>()) 
        {
            mesh->Render();
        }
    }
}

}
