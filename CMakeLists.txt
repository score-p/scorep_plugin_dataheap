project(scorep_dataheap_plugin)

cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -D_GLIBCXX_USE_NANOSLEEP")

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

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

add_library(dataheap_plugin SHARED src/main.cpp src/scoreptime.cpp)
target_link_libraries(dataheap_plugin ${LIBDATAHEAP_LIBRARIES})

install(TARGETS dataheap_plugin LIBRARY DESTINATION lib)
