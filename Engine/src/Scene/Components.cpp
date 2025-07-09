#include "Scene/Components.h"
#include "Scene/Actor.h"

namespace aero3d {

glm::mat4 SceneComponent::GetWorldTransform() const 
{
    if (m_Parent)
        return m_Parent->GetWorldTransform() * m_LocalTransform;
    return m_LocalTransform;
}

void SceneComponent::AttachTo(SceneComponent* parent) 
{
    if (m_Parent)
        Detach();
    m_Parent = parent;
    m_Parent->m_Children.push_back(this);
}

void SceneComponent::Detach() 
{
    if (!m_Parent) return;
    auto& siblings = m_Parent->m_Children;
    siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    m_Parent = nullptr;
}

void MeshComponent::Render() 
{

}

void CameraComponent::SetPerspective(float fov, float aspect, float nearClip, float farClip) 
{
    m_FOV = fov;
    m_AspectRatio = aspect;
    m_Near = nearClip;
    m_Far = farClip;
}

glm::mat4 CameraComponent::GetProjectionMatrix() const 
{
    return glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_Near, m_Far);
}

glm::mat4 CameraComponent::GetViewMatrix() const 
{
    glm::mat4 world = GetWorldTransform();
    glm::vec3 pos = glm::vec3(world[3]);
    glm::vec3 forward = glm::normalize(glm::vec3(world[2]));
    glm::vec3 up = glm::vec3(world[1]);

    return glm::lookAt(pos, pos - forward, up);
}

}
