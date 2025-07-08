#ifndef AERO3D_SCENE_COMPONENTS_H_
#define AERO3D_SCENE_COMPONENTS_H_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace aero3d {

struct TransformComponent 
{
    glm::vec3 position {0.0f};
    glm::quat rotation {1, 0, 0, 0};
    glm::vec3 scale {1.0f};
};

} // namespace aero3d

#endif // AERO3D_SCENE_COMPONENTS_H_