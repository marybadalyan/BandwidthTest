cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Tryout)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add threading support
find_package(Threads REQUIRED)

# Add the executable
add_executable(Tryout main.cpp)

# Link threading library
target_link_libraries(Tryout PRIVATE Threads::Threads)