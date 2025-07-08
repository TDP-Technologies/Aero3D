#ifndef AERO3D_SCENE_COMPONENTS_H_
#define AERO3D_SCENE_COMPONENTS_H_

namespace aero3d {

class GameObject;

class Component 
{
public:
    virtual ~Component() = default;

    virtual void OnAttach() = 0;
    virtual void OnDetach() = 0;
    virtual void Update(float deltaTime) = 0;
    
    void SetOwner(GameObject* gameObject) { owner = gameObject; }
    GameObject* GetOwner() { return owner; }

public:
    int updateOrder = 100;
    
protected:
    GameObject* owner = nullptr;

};

} // namespace aero3d

#endif // AERO3D_SCENE_COMPONENTS_H_