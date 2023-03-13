# Dev Log (ezg_gl_renderer)

#### Future plans (Phase0 - Phase9):
1. Phase 0: OpenGL set up
2. Phase 1: A render system
3. Phase 2: PBR
4. Phase 3: Image Based Lighting
5. Phase 4: Integrating ImGUI.
6. Phase 5: Shadows
7. Phase 6: Raytracing tracing
8. Phase 7: Support gltf features and extensions

#### Long-term goals
1. **read SIGGRAPH papers and try to implement them.**

--------

## 2023.2.12

#### Current Progress (Phase 0 Completed):

Now we have a simple OpenGL renderer, which is capable of:

1. loading gltf models and render (base color texture only).
2. moving around the scene through camera system. 

It has wrappers of OpenGL Context Objects such as:
1. Vertex Arrays
2. Buffers - Vertex buffer, Index buffer, Uniform buffer
3. Framebuffer objects
4. Texture
5. ShaderProgram




## 2023.2.17
#### Current Progress (Phase1 completed)\
I have built a basic renderer using 2 render targets, first pass write color to a texture and second pass draw a full-screen squad to default framebuffer.

#### Next stage plan:
load all materials and implement PBR.

## 2023.2.21
#### Struggling with color spaces
**Read specification!!!**\
**Final output**: FRAMEBUFFER_SRGB & sue GL_SGB8_ALPHA8 format color attachment for framebuffer
> OpenGL 4.6 spec cor
> 17.3.7 sRGB Conversion\
> If FRAMEBUFFER_SRGB is enabled and the value of FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING for the framebuffer attachment corresponding to the destination buffer is SRGB

**What about input?**
Some say read texture as sRGB textures by setting the texture format to GL_SGB8_ALPHA8 (or GL_SGB8), which avoid manual linearization of values fetched from sRGB images. \
**But** according to gltf specifications, baseColorTexture and emissiveTexture are encoded with the sRGB transfer function, while ohers are linear textures. So we can't convert them all using this method. \
My solution is: read using linear RGBA textures, and apply sRGBToLinear on my own.

#### Current Progress (Phase2 completed) 
#### Next stage plan:
1. Need to dive into PBR theories and figure out how it works.
2. Start implementing IBL.

## 2023.3.13

#### Current Progress (Phase3 & Phase4 completed)

#### Next stage plan:

1. Implement shadow mapping.
2. Implement cascade shadow mapping.