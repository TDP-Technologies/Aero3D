#ifndef AERO3D_SCENE_COMPONENTS_H_
#define AERO3D_SCENE_COMPONENTS_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>

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

class Mesh;

class MeshComponent : public SceneComponent 
{
public:
    void Render();

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

enum class LightType 
{
    Directional,
    Point,
    Spot
};

class LightComponent : public SceneComponent 
{
public:
    void SetType(LightType type) { m_Type = type; }
    void SetColor(const glm::vec3& color) { m_Color = color; }
    void SetIntensity(float intensity) { m_Intensity = intensity; }

    LightType GetType() const { return m_Type; }
    glm::vec3 GetColor() const { return m_Color; }
    float GetIntensity() const { return m_Intensity; }

private:
    LightType m_Type = LightType::Directional;
    glm::vec3 m_Color = {1.0f, 1.0f, 1.0f};
    float m_Intensity = 1.0f;
};

class ScriptComponent : public Component 
{
public:
    virtual void OnCreate() {}
    virtual void OnDestroy() {}
    virtual void OnUpdate(float deltaTime) {}
};

} // namespace aero3d

#endif // AERO3D_SCENE_COMPONENTS_H_
