# CMakeLists.txt

# 如果需要，提供这个模块的选项配置
option(ENABLE_THIRD_PARTY "Enable third-party dependencies" ON)

if (ENABLE_THIRD_PARTY)
    # 第三方库路径
    message(STATUS, "${CMAKE_CURRENT_LIST_DIR}/include")

    set(MX_SDK_HOME $ENV{MX_SDK_HOME})
    set(THIRD_PARTY_INCLUDE_DIR 
        "${CMAKE_CURRENT_LIST_DIR}/include"
        # "${CMAKE_CURRENT_LIST_DIR}/include/opencv4"
        ${MX_SDK_HOME}/include
        # ${MX_SDK_HOME}/opensource/include
        # ${MX_SDK_HOME}/opensource/include/opencv4
        # ${MX_SDK_HOME}/opensource/include/gstreamer-1.0
        # ${MX_SDK_HOME}/opensource/include/glib-2.0
        # ${MX_SDK_HOME}/opensource/lib/glib-2.0/includ

    )
    set(THIRD_PARTY_LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/lib")
    message(STATUS, "${THIRD_PARTY_INCLUDE_DIR}")
    # 添加头文件搜索路径
    include_directories(${THIRD_PARTY_INCLUDE_DIR})

    # 添加库文件路径
    link_directories(${THIRD_PARTY_LIB_DIR})

    # 如果有多个第三方库，可以逐一添加
    # 第三方库的名称根据实际情况调整
    # set(THIRD_PARTY_LIBS 
    #     some_library_name
    #     another_library_name
    # )

    # 将第三方库暴露为变量，供主项目使用
    set(THIRD_PARTY_INCLUDE_DIR ${THIRD_PARTY_INCLUDE_DIR} )
    set(THIRD_PARTY_LIB_DIR ${THIRD_PARTY_LIB_DIR} )
    set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} )
endif ()
