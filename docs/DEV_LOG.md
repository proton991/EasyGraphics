# Dev Log

## 2022.11.16

#### Current Progress:

Built a basic renderer by following [Vulkan Guide](https://vkguide.dev/)' chapter 0 through chapter 5. Already covered content

#### Next, continue to improve the engine.

**Reference projects:**

1. Vulkan Guide Engine: https://github.com/vblanco20-1/vulkan-guide
2. Granite: https://github.com/Themaister/Granite
3. VulkanSceneGraph: https://github.com/vsg-dev/VulkanSceneGraph
4. vulkan-renderer: https://github.com/inexorgame/vulkan-renderer

#### Start Phase1

Currently **`vk_helper`** module achieved some level of abstractions on Vulkan API, but not enough.  The target is to achieve a **mid-level** abstraction for `vk_helper` module, and use it to build the engine, finally achieving a high-level abstraction.  In the end, the engine V1.0 should look like this(approximately):

1. Support multiple model&texture format rendering.
2. Support multiple material rendering.
3. Support shadowing mapping and raytracing.
4. Basic control system: move camera and move objects.
5. Profiling: monitor fps and frame time.
6. UI: integrate imgui.

**Mark current version as V0.1**



## 2022.11.20 (Phase1)

#### Current Progress:

Implement `Context` abstraction layer by referencing project [Granite](https://github.com/Themaister/Granite).

`Context`is responsible for:

1. Create Instance.
2. Create Device.
3. Set up debugUtil Messenger.

Other progress:

1. Integrated logging system: [spdlog](https://github.com/gabime/spdlog).
2. Integrated [volk ](https://github.com/gnuradio/volk)to load Vulkan functions dynamically.

#### Next:
**New:**
1. Implement `WSI` abstraction layer that manages things like window creation, surface creation, frame operations...
    > **Note** For now just implemented a simple WSI abstraction which is responsible for surface creation and destruction - 2022.11.23
2. Implement `Swaphchain` abstraction
3. Implement a `RenderStage` abstraction layer which manages renderpass and rendering resources
4. Implement `Device` class which includes operation about `VkDevice`. 
   > It's the largest part, for it will include functionalities related to VkDevice. Build this part step by step
5. Implement asset-loader. (lower priority)

**Refactor**:
1. Remove `VkSurfaceKHR` dependency when initializing `vk::Context`, check present support when building swapchain