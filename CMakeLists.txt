cmake_minimum_required(VERSION 3.20)
project(paraflop VERSION 0.1.0)

set(CMAKE_BUILD_TYPE Debug)

# add_subdirectory(${CMAKE_SOURCE_DIR}/external/tinygltf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)
set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++20)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external/ktx/include)
include_directories(${CMAKE_SOURCE_DIR}/external/ktx/utils)
include_directories(${CMAKE_SOURCE_DIR}/external/tinygltf)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm CONFIG REQUIRED)

file(GLOB BASE_SRC *.cpp *.hpp *.h ../external/imgui/*.cpp)
file(GLOB BASE_HEADERS *.hpp *.h)

IF (NOT Vulkan_FOUND)
    message(FATAL_ERROR Could not find Vulkan library!)
ELSE()
    message(STATUS ${Vulkan_LIBRARY})
ENDIF()

find_program(Vulkan_GLSC_VALIDATOR
	NAMES glslc
	HINTS ENV VULKAN_SDK 
    PATH_SUFFIXES bin
)

if (NOT Vulkan_GLSC_VALIDATOR)
    message(FATAL_ERROR glslc not found!)
endif()

if(MSVC)
    add_compile_options(/W4 /Ox)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Ofast -ffast-math -march=native -g)
endif()

# Compile the dependencies
file(COPY 
    ${CMAKE_SOURCE_DIR}/assets/shaders
    ${CMAKE_SOURCE_DIR}/assets/textures
    DESTINATION
    ${CMAKE_BINARY_DIR})

file(GLOB BASE_SOURCES ${CMAKE_SOURCE_DIR}/src/base/*.cpp ${CMAKE_SOURCE_DIR}/include/*)

add_library(base ${BASE_SOURCES})

file(GLOB GLTF_MODEL_SOURCES ${CMAKE_SOURCE_DIR}/src/gltf_model/*.cpp ${CMAKE_SOURCE_DIR}/include/gltf_model/*)

add_library(gltf_model ${GLTF_MODEL_SOURCES})

# Complie the application
file(GLOB SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/include/*)

set(APP_SOURCE_FILES ${SOURCE_FILES} CACHE INTERNAL STRINGS)

add_executable(paraflop ${APP_SOURCE_FILES})

target_link_libraries(paraflop PUBLIC base)
target_link_libraries(paraflop PUBLIC Vulkan::Vulkan)
target_link_libraries(paraflop PUBLIC glfw)
target_link_libraries(paraflop PUBLIC glm::glm)
target_link_libraries(paraflop PUBLIC ktx)
target_link_libraries(paraflop PUBLIC gltf_model)
