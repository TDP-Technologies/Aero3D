#ifndef AERO3D_SCENE_SCENE_H_
#define AERO3D_SCENE_SCENE_H_

#include <vector>
#include <string>

#include "Scene/GameObject.h"

namespace aero3d {

class Scene
{
public:
    Scene();
    ~Scene();

    void Update(float deltaTime);

    void AddGameObject(GameObject* gameObject);

	void RemoveGameObject(std::string name);
    GameObject* GetGameObject(std::string name);

private:
    std::vector<GameObject*> m_GameObjects;

};

} // namespace aero3d

#endif // AERO3D_SCENE_SCENE_H_