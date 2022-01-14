cmake_minimum_required(VERSION 3.0.0)
project(levi VERSION 0.1.0)

include(CTest)
include_directories(include)
include_directories(src)
include_directories(debug)
enable_testing()


file(GLOB SOURCE_FILES src/*.cc)
add_executable(levi ${SOURCE_FILES} debug/debug.cc)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
