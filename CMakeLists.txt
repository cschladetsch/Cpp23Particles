cmake_minimum_required(VERSION 3.16)
project(ParticleSystem VERSION 0.1.0 LANGUAGES CXX)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add local paths for finding packages
list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}/particle_project/deps")

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Collect source files
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.hpp")

# Create executable
add_executable(particle_system ${SOURCES} ${HEADERS})

# Link SDL2
target_link_libraries(particle_system PRIVATE ${SDL2_LIBRARIES})

# Add compiler flags
target_compile_options(particle_system PRIVATE -Wall -Wextra -std=c++2b)
