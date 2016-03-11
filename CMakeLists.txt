cmake_minimum_required(VERSION 2.8)
project(dataheap_plugin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -D_GLIBCXX_USE_NANOSLEEP")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/common;${CMAKE_MODULE_PATH}")

set(PLUGIN_SOURCE src/main.cpp src/scoreptime.cpp)


include(common/FindScorep.cmake)
if(SCOREP_FOUND)
    include_directories(${SCOREP_INCLUDE_DIRS})
    link_directories(${SCOREP_LIBRARY_DIRS})
else()
    message(SEND_ERROR "Scorep was not found but is required!")
endif()


include(common/FindLibDataheap.cmake)
if(LIBDATAHEAP_FOUND)
    include_directories(${LIBDATAHEAP_INCLUDE_DIRS})
    link_directories(${LIBDATAHEAP_LIBRARY_DIRS})
else()
    message(SEND_ERROR "libdataheap was not found but is required!")
endif()


include_directories(include)
add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCE})
target_link_libraries(${PROJECT_NAME} ${LIBDATAHEAP_LIBRARIES})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
