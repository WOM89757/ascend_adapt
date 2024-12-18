cmake_minimum_required(VERSION 3.5.2)
project(yolov8posepostprocess)

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
add_definitions(-Dgoogle=mindxsdk_private)

set(PLUGIN_NAME "yolov8posepostprocess")
set(TARGET_LIBRARY ${PLUGIN_NAME})

include_directories(${MX_SDK_HOME}/include)
include_directories(${MX_SDK_HOME}/opensource/include)
include_directories(${MX_SDK_HOME}/opensource/include/gstreamer-1.0)
include_directories(${MX_SDK_HOME}/opensource/include/glib-2.0)
include_directories(${MX_SDK_HOME}/opensource/lib/glib-2.0/include)

link_directories(${MX_SDK_HOME}/opensource/lib/)
link_directories(${MX_SDK_HOME}/lib)

add_compile_options(-fPIC -std=c++14 -fstack-protector-all -Wl, -z,relro,-z,now,-z,noexecstack -s -pie)
add_compile_options("-DPLUGIN_NAME=${PLUGIN_NAME}")

add_definitions(-DENABLE_DVPP_INTERFACE)
add_library(${TARGET_LIBRARY} SHARED Yolov8PosePostProcess.cpp)

target_link_libraries(${TARGET_LIBRARY} glib-2.0 gstreamer-1.0 gobject-2.0 gstbase-1.0 gmodule-2.0)
target_link_libraries(${TARGET_LIBRARY} plugintoolkit mxpidatatype mxbase)
target_link_libraries(${TARGET_LIBRARY} -Wl,-z,relro,-z,now,-z,noexecstack -s)


install(FILES Yolov8PosePostProcess.h DESTINATION ${MX_SDK_HOME}/include/MxPlugins/ModelPostProcessors/)
install(TARGETS ${TARGET_LIBRARY} PERMISSIONS OWNER_READ GROUP_READ LIBRARY DESTINATION ${MX_SDK_HOME}/lib/modelpostprocessors)