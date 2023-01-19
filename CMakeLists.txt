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

# spdlog
    find_package(spdlog REQUIRED)
    include_directories(${SPDLOG_INCLUDE_DIRS})
    link_libraries(${SPDLOG_LIBRARIES})
#

# csv2
    include_directories(Vendor/csv2/include)
#

# CWG
    link_libraries(spdlog SDL2 SDL2_image)

    file(GLOB CWG Source/*.cpp Source/Include/*.hpp)

    add_executable(CWG ${CWG})
    include_directories(Source/Include)
    add_compile_definitions(_USE_MATH_DEFINES)
#
