find_package(Vulkan REQUIRED)

#both vma and glm and header only libs so we only need the include path
###########################
#       vma               #
###########################
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE vma)

###########################
#       glm               #
###########################
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

###########################
#      tinyobjloader      #
###########################
add_library(tinyobjloader STATIC)
target_sources(tinyobjloader PRIVATE
    tinyobjloader/tiny_obj_loader.h
    tinyobjloader/tiny_obj_loader.cc
    )
target_include_directories(tinyobjloader PUBLIC tinyobjloader)

###########################
#        tinygltf         #
###########################
add_library(tinygltf STATIC)
target_sources(tinygltf PRIVATE
    tinygltf/tiny_gltf.h
    tinygltf/tiny_gltf.cc
    tinygltf/json.hpp
    tinygltf/stb_image.h
    tinygltf/stb_image_write.h
    )
target_include_directories(tinygltf PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/tinygltf")


###########################
#         json            #
###########################
add_library(json INTERFACE)
target_include_directories(json INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann_json")

###########################
#         sdl2            #
###########################
add_library(sdl2 INTERFACE)
set(sdl2_DIR ${CMAKE_SOURCE_DIR}/third_party/sdl2)
message(STATUS "SDL2: ${sdl2_DIR}")
if (WIN32)
  target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include ${sdl2_image_DIR}/include)
  target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib/x64 ${sdl2_image_DIR}/lib/x64)
elseif (UNIX)
  target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include/SDL2 ${sdl2_image_DIR}/include/SDL2)
  target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib ${sdl2_image_DIR}/lib)
endif ()
target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)

###########################
#         glfw            #
###########################
add_library(glfw INTERFACE)
set(GLFW_DIR ${CMAKE_SOURCE_DIR}/third_party/glfw-3.3.8)
target_include_directories(glfw INTERFACE ${GLFW_DIR}/include)
target_link_directories(glfw INTERFACE ${GLFW_DIR}/lib-vc2022)
target_link_libraries(glfw INTERFACE glfw3)

###########################
#         imgui           #
###########################
add_library(imgui STATIC)
target_include_directories(imgui PUBLIC imgui)
target_sources(imgui PRIVATE
    imgui/imgui.h
    imgui/imgui.cpp

    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp

    imgui/imgui_impl_vulkan.cpp
    imgui/imgui_impl_sdl.cpp
    )
target_link_libraries(imgui PUBLIC Vulkan::Vulkan sdl2)

###########################
#         glad            #
###########################
add_library(glad STATIC)
target_include_directories(glad PUBLIC glad/include)
target_sources(glad PRIVATE
    glad/include/glad/glad.h
    glad/include/KHR/khrplatform.h
    glad/src/glad.c
    )
target_include_directories(glad PUBLIC glad/include)

###########################
#         assimp          #
###########################
add_library(assimp INTERFACE)
set(ASSIMP_DIR ${CMAKE_SOURCE_DIR}/third_party/assimp)
target_include_directories(glfw INTERFACE ${ASSIMP_DIR}/include)
target_link_directories(glfw INTERFACE ${ASSIMP_DIR}/lib)
target_link_libraries(glfw INTERFACE assimp-vc143-mt)

###########################
#         stb_image       #
###########################
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE stb_image)

###########################
#         volk            #
###########################
# volk must be STATIC.
add_library(volk STATIC volk/volk.c volk/volk.h)
if (WIN32)
  target_compile_definitions(volk PRIVATE VK_USE_PLATFORM_WIN32_KHR)
else ()
  target_link_libraries(volk PRIVATE dl)
endif ()
target_include_directories(volk PUBLIC volk ${VULKAN_INCLUDE_DIR})

###########################
#         spdlog          #
###########################
add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE spdlog-1.11.0/include)