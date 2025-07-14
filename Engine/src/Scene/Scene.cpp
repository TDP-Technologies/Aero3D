#include "Scene/Scene.h"
#include "Scene/Actor.h"
#include "Scene/Components.h"

namespace aero3d {

std::unordered_map<std::size_t, std::function<std::unique_ptr<Actor>()>>  Scene::s_ActorRegistry;
std::unordered_map<std::size_t, std::function<std::unique_ptr<Component>()>> Scene::s_ComponentRegistry;

Scene::Scene()
{
    Scene::RegisterActor<Actor>();

    Scene::RegisterComponent<SceneComponent>();
    Scene::RegisterComponent<SpriteComponent>();
    Scene::RegisterComponent<CameraComponent>();
}

Scene::~Scene()
{

}

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
    }
}

}
