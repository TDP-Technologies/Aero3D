#ifndef AERO3D_SCENE_COMPONENTS_H_
#define AERO3D_SCENE_COMPONENTS_H_

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Graphics/Resources.h"

namespace aero3d {

class Actor;

class Component 
{
public:
    virtual ~Component() = default;
    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void Update(float deltaTime) {}

    void SetOwner(Actor* owner) { m_Owner = owner; }
    Actor* GetOwner() const { return m_Owner; }

protected:
    Actor* m_Owner = nullptr;
};

class SceneComponent : public Component 
{
public:
    void SetLocalTransform(const glm::mat4& transform) { m_LocalTransform = transform; }
    glm::mat4 GetLocalTransform() const { return m_LocalTransform; }
    glm::mat4 GetWorldTransform() const;

    void AttachTo(SceneComponent* parent);
    void Detach();

protected:
    SceneComponent* m_Parent = nullptr;
    std::vector<SceneComponent*> m_Children;
    glm::mat4 m_LocalTransform = glm::mat4(1.0f);
    
};

class SpriteComponent : public SceneComponent 
{
public:
    void SetTexture(Ref<TextureView> texture) { m_Texture = texture; }
    Ref<TextureView> GetTexture() { return m_Texture; }

private:
    Ref<TextureView> m_Texture = nullptr;

};

class CameraComponent : public SceneComponent 
{
public:
    void SetPerspective(float fov, float aspect, float nearClip, float farClip);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

private:
    float m_FOV = 60.0f;
    float m_AspectRatio = 16.0f / 9.0f;
    float m_Near = 0.1f;
    float m_Far = 1000.0f;
};

} // namespace aero3d

#endif // AERO3D_SCENE_COMPONENTS_H_
