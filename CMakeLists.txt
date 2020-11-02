cmake_minimum_required(VERSION 3.14)
project(OCPPPP VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0" CACHE STRING "" FORCE)

include(cmake/CPM.cmake)
set(CPM_USE_LOCAL_PACKAGES OFF)
set(CPM_LOCAL_PACKAGES_ONLY OFF)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts-diagnostics-depth=1000")
endif()
include(cmake/CPM.cmake)

#file(GLOB_RECURSE headers ${OCPPPP_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE sources ${OCPPPP_SOURCE_DIR}/src/*.cpp)

add_executable(ocpppp "${sources}")
target_include_directories(ocpppp PUBLIC "${OCPPPP_SOURCE_DIR}/src")
target_include_directories(ocpppp PUBLIC "${OCPPPP_SOURCE_DIR}/external/include")

add_library(ocpppp_lib INTERFACE)
target_include_directories(ocpppp_lib INTERFACE "${OCPPPP_SOURCE_DIR}/lib")

CPMAddPackage(
  NAME cppast
  GITHUB_REPOSITORY foonathan/cppast
  GIT_TAG master
)
target_link_libraries(ocpppp PUBLIC cppast)

CPMAddPackage(
  NAME cxxopts
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 2.2.1
)
target_link_libraries(ocpppp PUBLIC cxxopts)

# ocpppp_generate(generated "${OCPPPP_SOURCE_DIR}/tests" "test1.cpp")
