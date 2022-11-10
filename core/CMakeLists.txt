file(GLOB vulkan_helper_src vulkan_helper/*.cpp vulkan_helper/*.hpp)
file(GLOB eg_engine_src eg_engine/*.cpp eg_engine/*.hpp)
add_library(vulkan_helper ${vulkan_helper_src})
add_library(eg_engine ${eg_engine_src})

target_link_libraries(eg_engine vma tinyobjloader sdl2 stb_image)
