cmake_minimum_required(VERSION 3.10)
project(C++Sample)

add_compile_options(-fPIC -std=c++14 -fstack-protector-all -Wl, -z,relro,-z,now,-z,noexecstack -s -pie)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0 -Dgoogle=mindxsdk_private)

set(MX_SDK_HOME $ENV{MX_SDK_HOME})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")

include_directories(
    ${MX_SDK_HOME}/include
    ${MX_SDK_HOME}/opensource/include
    ${MX_SDK_HOME}/opensource/include/opencv4
)

link_directories(
    ${MX_SDK_HOME}/lib
    ${MX_SDK_HOME}/include
    ${MX_SDK_HOME}/opensource/lib
    ${MX_SDK_HOME}/lib/modelpostprocessors
)

link_libraries(
  /usr/local/Ascend/ascend-toolkit/latest/lib64
)

add_subdirectory(plugins)

add_executable(yoloInfer main.cpp)
target_link_libraries(yoloInfer
    glog
    mxbase
    yolov7postprocess
    openposepostprocess
    yolov8posepostprocess
    opencv_world
)


# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

