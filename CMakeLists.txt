cmake_minimum_required(VERSION 3.23)

cmake_policy(SET CMP0126 NEW)
cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0102 NEW)

project(CWG)

set(EXPORT_COMPILE_COMMANDS true)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# SDL2
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    link_libraries(${SDL2_LIBRARIES})
#

# SDL2_image
    find_package(SDL2_image REQUIRED)
    include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
    link_libraries(${SDL2_IMAGE_LIBRARIES})
#

# SDL2_image
    find_package(SDL2_mixer REQUIRED)
    include_directories(${SDL2_MIXER_INCLUDE_DIRS})
    link_libraries(${SDL2_MIXER_LIBRARIES})
#

# CWG
    link_libraries(SDL2 SDL2_image SDL2_mixer)

    file(GLOB CWG Source/*.cpp Source/Include/*.hpp)

    add_executable(CWG ${CWG})

    target_precompile_headers(CWG PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:<CWGPCH.hpp$<ANGLE-R>>")
    include_directories(Source/Include)

    add_compile_definitions(_USE_MATH_DEFINES)
#
