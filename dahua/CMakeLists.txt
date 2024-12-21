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
find_library(cpprestsdk REQUIRED NAMES cpprestsdk cpprest)

add_subdirectory(safe_http)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries( ${PROJECT_NAME}
    # PRIVATE cpprestsdk::cpprest
    PRIVATE cpprest
    PRIVATE pthread ssl crypto
    PRIVATE cube-common-safe-http-object
    PRIVATE boost_system
    amqpcpp
    ev
)