#ifndef AERO3D_SCENE_GAMEOBJECT_H_
#define AERO3D_SCENE_GAMEOBJECT_H_

#include <string>
#include <vector>

#include "Scene/Components.h"

namespace aero3d {

class GameObject
{
public:
    GameObject(std::string objectName);
    ~GameObject();

    void Update(float deltaTime);
    void UpdateComponents(float deltaTime);

    virtual void UpdateGameObject(float deltaTime) {};

    void AddComponent(Component* component);
	void RemoveComponent(Component* component);

public:
    std::string name;

private:
    std::vector<Component*> m_Components;

};

} // namespace aero3d

#endif // AERO3D_SCENE_GAMEOBJECT_H_