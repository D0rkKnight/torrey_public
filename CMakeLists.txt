cmake_minimum_required(VERSION 3.10)

project(lajolla)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_BUILD_TYPE Debug)

if(MSVC)
add_compile_options(/Wall)
else()
add_compile_options(-Wall)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

# Set custom files as src as well
file(GLOB_RECURSE CUSTOM_SRCS ${CMAKE_SOURCE_DIR}/src/custom/*.cpp)

set(SRCS src/3rdparty/miniz.h
src/3rdparty/miniz.c
src/3rdparty/pugiconfig.hpp
         src/3rdparty/pugixml.hpp
         src/3rdparty/pugixml.cpp
         src/3rdparty/stb_image.h
         src/3rdparty/tinyexr.h
         src/3rdparty/tinyply.h
         src/compute_normals.h
         src/flexception.h
         src/hw1.h
         src/hw1_scenes.h
         src/hw2.h
         src/hw3.h
         src/hw4.h
         src/image.h
         src/matrix.h
         src/parallel.h
         src/parse_obj.h
         src/parse_ply.h
         src/parse_scene.h
         src/parse_serialized.h
         src/print_scene.h
         src/torrey.h
         src/transform.h
         src/vector.h
         src/compute_normals.cpp
         src/hw1.cpp
         src/hw2.cpp
         src/hw3.cpp
         src/hw4.cpp
         src/image.cpp
         src/main.cpp
         src/parallel.cpp
         src/parse_obj.cpp
         src/parse_ply.cpp
         src/parse_scene.cpp
         src/parse_serialized.cpp
         src/print_scene.cpp
         src/transform.cpp

        ${CUSTOM_SRCS}
        #  ${CUSTOM_HEADERS}
         )


add_library(torrey_lib STATIC ${SRCS})
add_executable(torrey src/main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(torrey Threads::Threads)

target_link_libraries(torrey torrey_lib)

# # Nuke them warnings
# if(MSVC)
#     add_compile_options(/W0)
# elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
# add_compile_options(-w)
# endif()

# Build and link testing executable
# Turn off optimizations for testing

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/cb455a71fb23303e37ce8ee5b1cde6a2c18f66a5.zip
)
FetchContent_MakeAvailable(googletest)

# enable_testing()

file(GLOB_RECURSE GTEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/tests/*.cpp)

add_executable(
  gtests
  ${GTEST_SOURCES}
)
target_link_libraries(
  gtests
  GTest::gtest_main
)

target_link_libraries(gtests Threads::Threads)
target_link_libraries(gtests torrey_lib)

# Turn off optimizations for testing
# set_target_properties(gtests PROPERTIES COMPILE_FLAGS -O0)

include(GoogleTest)
gtest_discover_tests(gtests)