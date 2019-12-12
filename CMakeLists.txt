cmake_minimum_required(VERSION 3.8.2)
project(kopt)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-deprecated-declarations")

cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0076 NEW)

include(cmake/DownloadProject.cmake)
download_project(
    PROJ googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.1
    UPDATE_DISCONNECTED 1
)
add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
download_project(
    PROJ gflags
    GIT_REPOSITORY https://github.com/gflags/gflags.git
    GIT_TAG v2.2.2
    UPDATE_DISCONNECTED 1
)
add_subdirectory(${gflags_SOURCE_DIR} ${gflags_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(src)
target_link_libraries(clever_lib gflags::gflags)

add_executable(kopt "src/main.cpp")
target_link_libraries(kopt clever_lib gflags::gflags)

add_executable(gentest "src/gentest.cpp")
target_link_libraries(gentest clever_lib gflags::gflags)
