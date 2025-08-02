cmake_minimum_required(VERSION 4.0.0)
project(Bloom_CPP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# Find OpenMP
find_package(OpenMP REQUIRED)

# Define your executable
set(SRC_DIR src_claude_openmp) # Change this to compile other versions of the code
add_executable(Bloom_CPP ${SRC_DIR}/main.cpp ${SRC_DIR}/MyImage.cpp ${SRC_DIR}/MyImage.h)

target_include_directories(Bloom_CPP PRIVATE ${SRC_DIR})

# Link raylib
target_link_libraries(Bloom_CPP PRIVATE raylib)

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC optimizations
    target_compile_options(Bloom_CPP PRIVATE 
        -O3 
        -march=native 
        -ffast-math
        -funroll-loops
        -flto
    )
    target_link_options(Bloom_CPP PRIVATE -flto)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Clang optimizations
    target_compile_options(Bloom_CPP PRIVATE 
        -O3 
        -march=native 
        -ffast-math
        -funroll-loops
        -flto
    )
    target_link_options(Bloom_CPP PRIVATE -flto)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC optimizations
    target_compile_options(Bloom_CPP PRIVATE 
        /O2 
        # /arch:AVX2
        # /fp:fast
        # /GL
    )
    target_link_options(Bloom_CPP PRIVATE /LTCG)
endif()

# place the executable in the same folder as this CMakeLists.txt file
set_target_properties(Bloom_CPP PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}
)
# place the executable in the same folder as this CMakeLists.txt file
set_target_properties(Bloom_CPP PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}
)

# Print configuration info
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "OpenMP_CXX_VERSION: ${OpenMP_CXX_VERSION}")

# For Linux: Also link required system libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(Bloom_CPP PRIVATE m pthread dl rt X11)
endif()

# For OpenMP
# Link OpenMP if found
if(OpenMP_CXX_FOUND)
    target_link_libraries(Bloom_CPP PUBLIC OpenMP::OpenMP_CXX)
endif()
