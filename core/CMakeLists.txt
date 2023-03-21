find_package(Vulkan REQUIRED)
message(STATUS "Vulkan Include Dir: ${VULKAN_INCLUDE_DIR}")
message(STATUS "Vulkan Static Lib: ${Vulkan_LIBRARY}")
file(GLOB vulkan_helper_src vulkan_helper/*.cpp vulkan_helper/*.hpp)
file(GLOB ezg_engine_src ezg_engine/*.cpp ezg_engine/*.hpp)
file(GLOB ezg_vk_src ezg_vk/*.cpp ezg_vk/*.hpp)
file(GLOB ezg_vk_hpp_src ezg_vk_hpp/*.cpp ezg_vk_hpp/*.hpp ezg_util/*.hpp ezg_util/*.cpp)
file(GLOB ezg_asset_src ezg_asset/*.cpp ezg_asset/*.hpp)
file(GLOB_RECURSE ezg_gl_renderer_src ezg_gl_renderer/*.cpp ezg_gl_renderer/*.hpp)

add_library(vulkan_helper ${vulkan_helper_src})
add_library(ezg_engine ${ezg_engine_src})
add_library(ezg_vk ${ezg_vk_src})
add_library(ezg_vk_hpp ${ezg_vk_hpp_src})
add_library(ezg_asset ${ezg_asset_src})

target_link_libraries(ezg_engine vma tinyobjloader sdl2 stb_image ezg_asset spdlog)
target_link_libraries(ezg_vk volk spdlog sdl2 vma)
target_link_libraries(ezg_vk_hpp vma spdlog glfw ${Vulkan_LIBRARIES}) #

target_include_directories(ezg_engine PUBLIC ${VULKAN_INCLUDE_DIR})
target_include_directories(ezg_vk_hpp PUBLIC ${VULKAN_INCLUDE_DIR})
target_compile_definitions(ezg_vk_hpp PRIVATE EZG_DEBUG)

target_include_directories(vulkan_helper PUBLIC ${VULKAN_INCLUDE_DIR})

message(STATUS "ezg_gl_renderer_src: ${ezg_gl_renderer_src}")
add_library(ezg_gl_renderer ${ezg_gl_renderer_src})
target_include_directories(ezg_gl_renderer PUBLIC ezg_gl_renderer/)
target_link_libraries(ezg_gl_renderer glad glfw opengl32 stb_image tinyobjloader tinygltf spdlog glm imgui)