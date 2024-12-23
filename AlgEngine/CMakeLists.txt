# Copyright [cube] [name of copyright owner]

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.10)
project("cubeAlServe")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加 cpprestsdk 库
# find_library(cpprestsdk REQUIRED NAMES cpprestsdk cpprest)
# find_library(opencv4 REQUIRED)

include_directories(
     "${CMAKE_CURRENT_LIST_DIR}/third_party//include"
)
# include(${CMAKE_SOURCE_DIR}/third_party/CMakeLists.txt)

add_subdirectory(serverEngineBase)
include_directories(${CUBE_COMMON_SERVER_INCLUDE})

add_subdirectory(fcgServer)

add_executable(${PROJECT_NAME} main.cpp)

# link_directories(/usr/local/Ascend/mxVision/opensource/lib/)

target_link_libraries( ${PROJECT_NAME}
    # PRIVATE cpprestsdk::cpprest
    PRIVATE cpprest
    PRIVATE pthread  crypto
    # ssl
    PRIVATE ${CUBE_COMMON_SERVER_LIBRARY}
    PRIVATE ${CUBE_FCG_SERVER_LIBRARY}
    PRIVATE boost_system
    amqpcpp
    ev
)