# Dev Log

## 2022.11.16

#### Current Progress:

Built a basic renderer by following [Vulkan Guide](https://vkguide.dev/)' chapter 0 through chapter 5. Already covered content

#### Next, continue to improve the engine.

Reference projects:

1. Vulkan Guide Engine: https://github.com/vblanco20-1/vulkan-guide
2. Granite: https://github.com/Themaister/Granite
3. VulkanSceneGraph: https://github.com/vsg-dev/VulkanSceneGraph

#### Start Phase1

Currently **`vk_helper`** module achieved some level of abstractions on Vulkan API, but not enough.  The target is to achieve a **mid-level** abstraction for `vk_helper` module, and use it to build the engine, finally achieving a high-level abstraction.  In the end, the engine V1.0 should look like this(approximately):

1. Support multiple model&texture format rendering.
2. Support multiple material rendering.
3. Support shadowing mapping and raytracing.
4. Basic control system: move camera and move objects.
5. Profiling: monitor fps and frame time.
6. UI: integrate imgui.

**Mark current version as V0.1**