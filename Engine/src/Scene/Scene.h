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

private:
    std::vector<std::unique_ptr<Actor>> m_Actors;

};

} // namespace aero3d

#endif // AERO3D_SCENE_SCENE_H_
