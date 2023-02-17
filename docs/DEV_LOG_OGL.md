# Dev Log (ezg_gl_renderer)

#### Future plans (Phase0 - Phase4):
1. Phase 0: OpenGL set up
2. Phase 1: A render system
3. Phase 2: PBR
4. Phase 3: Render pipeline set up
5. Phase 4: Integrating ImGUI.


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
#### Current Progress (Phase1 complete)\
I have built a basic renderer using 2 render targets, first pass write color to a texture and second pass draw a full-screen squad to default framebuffer.

#### Next stage plan:
load all materials and implement PBR.