cmake_minimum_required(VERSION 3.27.7)

project(augustus VERSION 0.01)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS OFF)

#set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

add_executable(augustus)

if(MSVC)
  target_compile_options(augustus PRIVATE /W4)
else()
  target_compile_options(augustus PRIVATE -Wall -Wpedantic)
endif()

set(augustus_CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB augustus_src
	${augustus_CURRENT_DIR}/src/*.cpp
	${augustus_CURRENT_DIR}/src/*.c
)

file(GLOB rlImGui_src
    ${augustus_CURRENT_DIR}/deps/rlImGui/*.cpp
)

target_sources(augustus PRIVATE ${augustus_src} ${rlImGui_src})

target_include_directories(augustus PUBLIC
    ${augustus_CURRENT_DIR}/include
    ${augustus_CURRENT_DIR}/deps/rlImGui
)

add_subdirectory(${augustus_CURRENT_DIR}/deps/raylib)
add_subdirectory(${augustus_CURRENT_DIR}/deps/box2c)
add_subdirectory(${augustus_CURRENT_DIR}/deps/cimgui)

target_link_libraries(augustus PUBLIC
    raylib
    box2d
    cimgui
)

add_custom_command(
    TARGET augustus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${augustus_CURRENT_DIR}/resources
            ${CMAKE_CURRENT_BINARY_DIR}/resources)
