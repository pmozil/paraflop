cmake_minimum_required(VERSION 3.15)
project(paraflop VERSION 0.1.0)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")
set(CMAKE_BINARY_DIR "${PROJECT_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread")

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm CONFIG REQUIRED)

IF (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
    message(STATUS ${Vulkan_LIBRARY})
ENDIF()

find_program(Vulkan_GLSC_VALIDATOR
	NAMES glslc
	HINTS ENV VULKAN_SDK 
    PATH_SUFFIXES bin
)

if (NOT Vulkan_GLSC_VALIDATOR)
    message(FATAL_ERROR "glslc not found!")
endif()

if(MSVC)
    add_compile_options(/W4 /Ox)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Ofast -ffast-math -march=native -g)
endif()

file(COPY 
    "${CMAKE_SOURCE_DIR}/assets/shaders/frag.spv"
    "${CMAKE_SOURCE_DIR}/assets/shaders/vert.spv"
    DESTINATION
    "${CMAKE_BINARY_DIR}")

include_directories("${CMAKE_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/external/stb")

file(GLOB SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/include/*")
set(APP_SOURCE_FILES ${SOURCE_FILES} CACHE INTERNAL STRINGS)

add_executable(paraflop ${APP_SOURCE_FILES})

target_link_libraries(paraflop PUBLIC Vulkan::Vulkan)
target_link_libraries(paraflop PUBLIC glfw)
target_link_libraries(paraflop PUBLIC glm::glm)
