#ifndef AERO3D_GRAPHICS_GRAPHICSPIPELINE_H_
#define AERO3D_GRAPHICS_GRAPHICSPIPELINE_H_

namespace aero3d {

class GraphicsPipeline
{
public:
    virtual ~GraphicsPipeline() = default;

    virtual void Bind() = 0;
    virtual void Unbind() = 0;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_GRAPHICSPIPELINE_H_