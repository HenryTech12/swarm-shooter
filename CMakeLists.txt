cmake_minimum_required(VERSION 3.14)
project(swarm_shooter)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_executable(swarm_shooter main.cpp
)
target_link_libraries(swarm_shooter raylib)

if (WIN32)
    target_link_libraries(swarm_shooter opengl32 gdi32 winmm)
endif()