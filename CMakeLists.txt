cmake_minimum_required(VERSION 3.10)
project(GHandWrite VERSION 1.0 LANGUAGES CXX C) # Added 'C' here too

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)
find_package(CURL REQUIRED)

pkg_check_modules(EVDEV REQUIRED libevdev)
pkg_check_modules(SDL2_GFX REQUIRED SDL2_gfx)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

add_executable(${PROJECT_NAME} 
    src/input.cpp
    src/main.cpp
    src/network.cpp
    src/render.cpp
    src/log.c  
)

target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
set_source_files_properties(src/log.c PROPERTIES LANGUAGE C)

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
    rt # Link real-time library for clock_gettime in log.c
)
