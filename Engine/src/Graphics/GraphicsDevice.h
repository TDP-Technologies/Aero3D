#ifndef AERO3D_GRAPHICS_GRAPHICSDEVICE_H_
#define AERO3D_GRAPHICS_GRAPHICSDEVICE_H_

#include "Graphics/CommandList.h"
#include "Graphics/Resources.h"
#include "Graphics/Swapchain.h"
#include "Graphics/ResourceFactory.h"
#include "Utils/Common.h"

class SDL_Window;

namespace aero3d {

enum class RenderingAPI
{
    DirectX12,
    Vulkan
};

struct RenderSurfaceCreateInfo 
{
    enum class WindowType 
    {
        SDL,
        Win32,
        X11,
        Wayland,
        Cocoa,
        Unknown
    } type;

    union 
    {
        SDL_Window* sdlWindow;
        struct {
            void* hinstance;
            void* hwnd;
        } win32;
        struct {
            void* display;
            unsigned long window;
        } x11;
    };
};

class GraphicsDevice 
{
public:
    virtual ~GraphicsDevice() = default;

    virtual Ref<CommandList> CreateCommandList() = 0;
    virtual ResourceFactory* GetResourceFactory() = 0;
    virtual Swapchain* GetSwapchain() = 0;

    virtual void SubmitCommands(Ref<CommandList> commandList) = 0;
    virtual void Present() = 0;

    virtual void UpdateBuffer(Ref<DeviceBuffer> buffer, void* data, size_t size, size_t offset = 0) = 0;
    virtual void UpdateTexture(Ref<Texture> texture, void* data, size_t size) = 0;

};


} // namespace aero3d

#endif // AERO3D_GRAPHICS_GRAPHICSDEVICE_H_