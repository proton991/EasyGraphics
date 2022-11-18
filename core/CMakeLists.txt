file(GLOB vulkan_helper_src vulkan_helper/*.cpp vulkan_helper/*.hpp)
file(GLOB ezg_engine_src ezg_engine/*.cpp ezg_engine/*.hpp)
file(GLOB ezg_vk_src ezg_vk/*.cpp ezg_vk/*.hpp)
add_library(vulkan_helper ${vulkan_helper_src})
add_library(ezg_engine ${ezg_engine_src})
add_library(ezg_vk ${ezg_vk_src})

target_link_libraries(ezg_engine vma tinyobjloader sdl2 stb_image)