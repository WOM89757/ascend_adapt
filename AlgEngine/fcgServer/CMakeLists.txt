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

file(GLOB_RECURSE SOURCES *.cpp *.cc *.c)

set(INCLUDE ${CMAKE_CURRENT_LIST_DIR})

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${INCLUDE})

set(LIBRARY cube-fcg-server-object)
add_library(${LIBRARY} SHARED ${SOURCES})
set_property(TARGET ${LIBRARY} PROPERTY POSITION_INDEPENDENT_CODE ON)

set(CUBE_FCG_SERVER_LIBRARY ${LIBRARY} CACHE INTERNAL "")
set(CUBE_FCG_SERVER_INCLUDE ${INCLUDE} CACHE INTERNAL "")