cmake_minimum_required(VERSION 3.8)
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
project(EasyGraphics VERSION 0.1)

############################################################
#                    Include Directories                   #
############################################################
set(VULKAN_INCLUDE_DIR $ENV{VULKAN_SDK}/Include)
#include_directories(${VULKAN_INCLUDE_DIR})
include_directories(core)
#include_directories(third_party)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Use FindVulkan module added with CMAKE 3.7
#if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
#  message(STATUS "Using module to find Vulkan")
#  find_package(Vulkan)
#endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_definitions(-DVK_ASSETS_DIR=\"${CMAKE_SOURCE_DIR}/assets/\")
add_definitions(-DVK_SHADERS_DIR=\"${CMAKE_SOURCE_DIR}/shaders/spv/\")


find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
if (NOT GLSL_VALIDATOR)
  message(FATAL_ERROR "glslangValidator not found!")
endif ()

############################################################
#                    Add Sub Directories                   #
############################################################
add_subdirectory(third_party)
add_subdirectory(core)
#add_subdirectory(src)
add_subdirectory(app)


############################################################
#                    Build Shaders                         #
############################################################

file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${PROJECT_SOURCE_DIR}/shaders/*.frag"
  "${PROJECT_SOURCE_DIR}/shaders/*.vert"
  "${PROJECT_SOURCE_DIR}/shaders/*.comp"
  )

foreach (GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/spv/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} --target-env spirv1.3 -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(
  Shaders
  DEPENDS ${SPIRV_BINARY_FILES}
)