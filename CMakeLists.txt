cmake_minimum_required(VERSION 3.10)
project(GHandWrite VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find PkgConfig to handle libevdev and SDL2_gfx easily
find_package(PkgConfig REQUIRED)

#  Dependencies
find_package(SDL2 REQUIRED)
find_package(CURL REQUIRED)

# Use PkgConfig for libevdev and SDL2_gfx (often easier for these specific libs)
pkg_check_modules(EVDEV REQUIRED libevdev)
pkg_check_modules(SDL2_GFX REQUIRED SDL2_gfx)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

add_executable(${PROJECT_NAME} 
src/input.cpp
src/input.hpp
src/main.cpp
src/network.cpp
src/network.hpp
src/render.cpp
src/render.hpp
src/types.hpp

)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${SDL2_INCLUDE_DIRS} 
    ${CURL_INCLUDE_DIRS}
    ${EVDEV_INCLUDE_DIRS}
    ${SDL2_GFX_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    SDL2::SDL2 
    ${CURL_LIBRARIES}
    ${EVDEV_LIBRARIES}
    ${SDL2_GFX_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
)

target_compile_options(${PROJECT_NAME} PRIVATE -pg)
target_link_options(${PROJECT_NAME} PRIVATE -pg)
