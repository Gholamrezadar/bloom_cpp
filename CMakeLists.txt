cmake_minimum_required(VERSION 4.1.0)
project(Bloom_CPP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch raylib from GitHub
include(FetchContent)

set(RAYLIB_VERSION 5.0)

FetchContent_Declare(
        raylib
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
        FIND_PACKAGE_ARGS
)

# Let raylib build as a static library
set(BUILD_EXAMPLES OFF)
set(BUILD_GAMES OFF)

FetchContent_MakeAvailable(raylib)

# Define your executable
add_executable(Bloom_CPP src/main.cpp)

# Link raylib
target_link_libraries(Bloom_CPP PRIVATE raylib)

# place the executable in the same folder as this CMakeLists.txt file
set_target_properties(Bloom_CPP PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# For Linux: Also link required system libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(Bloom_CPP PRIVATE m pthread dl rt X11)
endif()
