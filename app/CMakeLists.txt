# Function for building single example
function(buildApp APP_NAME)
  SET(APP_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME})
  message(STATUS "Generating project file for app in ${APP_FOLDER}")
  # Main
  file(GLOB APP_SOURCE ${APP_FOLDER}/*.cpp ${APP_FOLDER}/*.hpp)
  SET(MAIN_CPP ${APP_FOLDER}/${APP_NAME}.cpp)
  if (EXISTS ${APP_FOLDER}/main.cpp)
    SET(MAIN_CPP ${APP_FOLDER}/main.cpp)
  ENDIF ()
  add_executable(${APP_NAME} ${APP_SOURCE})
  add_dependencies(${APP_NAME} Shaders)
  target_link_libraries(${APP_NAME} vulkan_helper ezg_engine)
endfunction()

set(APPS
  CreateInstance
  CreateDevice
  CreateSwapchain
  FirstTriangle
)
add_subdirectory(EzgVulkan)

############################################################
#                    Build Apps                            #
############################################################
function(buildApps)
  foreach(APP ${APPS})
    buildApp(${APP})
  endforeach(APP)
endfunction(buildApps)
buildApps()