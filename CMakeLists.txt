cmake_minimum_required(VERSION 3.20)
project(EngineCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Werror -Wpedantic -Wshadow)
endif()

# Locate standard external library packages
find_package(SDL2 REQUIRED)
find_package(GTest REQUIRED)

# Target 1: The Main Visual 3D Game Engine Executable
add_executable(EngineApp src/main.cpp src/Engine.cpp)
target_include_directories(EngineApp PRIVATE include ${SDL2_INCLUDE_DIRS})
target_link_libraries(EngineApp PRIVATE ${SDL2_LIBRARIES})

# Target 2: The Automated Headless QA Verification Engine
add_executable(RunTests tests/test_math.cpp)
target_link_libraries(RunTests PRIVATE GTest::GTest GTest::Main)
