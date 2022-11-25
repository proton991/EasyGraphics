file(GLOB vulkan_helper_src vulkan_helper/*.cpp vulkan_helper/*.hpp)
file(GLOB ezg_engine_src ezg_engine/*.cpp ezg_engine/*.hpp)
file(GLOB ezg_vk_src ezg_vk/*.cpp ezg_vk/*.hpp)
file(GLOB ezg_asset_src ezg_asset/*.cpp ezg_asset/*.hpp)

add_library(vulkan_helper ${vulkan_helper_src})
add_library(ezg_engine ${ezg_engine_src})
add_library(ezg_vk ${ezg_vk_src})
add_library(ezg_asset ${ezg_asset_src})

target_link_libraries(ezg_engine vma tinyobjloader sdl2 stb_image ezg_asset spdlog)
target_link_libraries(ezg_vk volk spdlog sdl2)

target_include_directories(ezg_engine PUBLIC ${VULKAN_INCLUDE_DIR})
target_include_directories(ezg_vk PUBLIC ${VULKAN_INCLUDE_DIR})
target_include_directories(vulkan_helper PUBLIC ${VULKAN_INCLUDE_DIR})