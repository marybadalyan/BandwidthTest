cmake_minimum_required(VERSION 3.10)

project(Tryout)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Compiler flags for AVX2
if(MSVC)
    add_definitions(/arch:AVX2)
elseif(UNIX) # Covers Linux and macOS
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
endif()

# Add executable
add_executable(Tryout main.cpp)

# Ensure AVX2 support
target_compile_options(Tryout PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-mavx2>
)