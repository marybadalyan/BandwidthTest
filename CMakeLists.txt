cmake_minimum_required(VERSION 3.10)
project(Tryout)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/arch:AVX2)
elseif(UNIX)
    add_compile_options(-mavx2)
endif()

find_package(Threads REQUIRED)
add_executable(Tryout main.cpp)
target_link_libraries(Tryout PRIVATE Threads::Threads)

set_target_properties(Tryout PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)