cmake_minimum_required(VERSION 2.8)
project(dataheap_plugin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -D_GLIBCXX_USE_NANOSLEEP")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/common;${CMAKE_MODULE_PATH}")

include(common/DefaultBuildType.cmake)

find_package(Scorep REQUIRED)
include(scorep/ScorepCXXPlugin.cmake)

find_package(LibDataheap REQUIRED)
if(LIBDATAHEAP_FOUND)
    include_directories(${LIBDATAHEAP_INCLUDE_DIRS})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(include)

add_library(dataheap_cxx_core STATIC src/dataheap_core.cpp)
set_target_properties(dataheap_cxx_core PROPERTIES COMPILE_FLAGS "-fPIC")
target_link_libraries(dataheap_cxx_core ${LIBDATAHEAP_LIBRARIES})

add_library(${PROJECT_NAME} SHARED src/main.cpp)
target_link_libraries(${PROJECT_NAME} dataheap_cxx_core)

add_executable(dataheap_interface_test src/dataheap_test.cpp)
target_link_libraries(dataheap_interface_test dataheap_cxx_core)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
