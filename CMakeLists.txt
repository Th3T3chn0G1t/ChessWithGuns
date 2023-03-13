cmake_minimum_required(VERSION 3.23)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

cmake_policy(SET CMP0126 NEW)
cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0102 NEW)

project(CWG LANGUAGES CXX OBJC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# SDL
    set(SDL3_SUBPROJECT ON)
    set(SDL_STATIC ON)
    set(SDL_SHARED OFF)
    set(SDL_ATOMIC OFF)
    set(SDL_JOYSTICK OFF)
    set(SDL_HAPTIC OFF)
    set(SDL_HIDAPI OFF)
    set(SDL_POWER OFF)
    set(SDL_FILE OFF)
    set(SDL_TIMERS OFF)
    set(SDL_FILESYSTEM OFF)
    set(SDL_SENSOR OFF)
    set(SDL_LOCALE OFF)
    set(SDL_MISC OFF)
    set(SDL_TEST OFF)
    # set(SDL_FRAMEWORK ON)
    add_subdirectory(${CMAKE_SOURCE_DIR}/Vendor/SDL)
#

# SDL_image
    set(SDL3IMAGE_BUILD_SHARED_LIBS OFF)
    set(SDL3IMAGE_DISABLE_INSTALL ON)
    set(SDL3IMAGE_INSTALL OFF)
    set(BUILD_SHARED_LIBS OFF)
    set(SDL3IMAGE_AVIF OFF)
    set(SDL3IMAGE_BMP OFF)
    set(SDL3IMAGE_GIF OFF)
    set(SDL3IMAGE_JPG OFF)
    set(SDL3IMAGE_JXL OFF)
    set(SDL3IMAGE_LBM OFF)
    set(SDL3IMAGE_PCX OFF)
    set(SDL3IMAGE_PNG ON)
    set(SDL3IMAGE_PNM OFF)
    set(SDL3IMAGE_QOI OFF)
    set(SDL3IMAGE_SVG OFF)
    set(SDL3IMAGE_TGA OFF)
    set(SDL3IMAGE_TIF OFF)
    set(SDL3IMAGE_WEBP OFF)
    set(SDL3IMAGE_XCF OFF)
    set(SDL3IMAGE_XPM OFF)
    set(SDL3IMAGE_XV OFF)
    add_subdirectory(${CMAKE_SOURCE_DIR}/Vendor/SDL_image)
    set(SDL3_DIR ${CMAKE_SOURCE_DIR}/Vendor/SDL_image)
#

# SDL_mixer
    set(SDL3MIXER_INSTALL OFF)
    set(SDL3MIXER_BUILD_SHARED_LIBS OFF)
    set(SDL3MIXER_CMD OFF)
    set(SDL3MIXER_FLAC OFF)
    set(SDL3MIXER_SNDFILE OFF)
    set(SDL3MIXER_GME OFF)
    set(SDL3MIXER_MOD OFF)
    set(SDL3MIXER_MP3 OFF)
    set(SDL3MIXER_MIDI OFF)
    set(SDL3MIXER_OPUS OFF)
    set(SDL3MIXER_VORBIS OFF)
    set(SDL3MIXER_WAVPACK OFF)
    set(SDL3MIXER_OGG OFF)
    add_subdirectory(${CMAKE_SOURCE_DIR}/Vendor/SDL_mixer)
#

# CWG
    file(GLOB CWG Source/*.cpp Source/Include/*.hpp)

    if(${APPLE})
        link_libraries("-framework CoreFoundation" "-framework IOKit")
        add_executable(CWG MACOSX_BUNDLE ${CWG})
        set_target_properties(CWG PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "Chess with Guns"
            MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
            MACOSX_BUNDLE_COPYRIGHT "SPDX-License-Identifier: GPL-3.0-or-later
                Copyright (C) 2023 Emily \"TTG\" Banerjee (prs.ttg+cwg@pm.me)" # Can't use angle brackets here
            MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.th3t3chn0g1t.CWG"
            MACOSX_BUNDLE_ICON_FILE "Icon.icns"
            MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
        )
        add_custom_command(TARGET CWG POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Resources ${CMAKE_BINARY_DIR}/CWG.app/Contents/Resources
        )
    else()
        add_executable(CWG ${CWG})
    endif()

    target_link_libraries(CWG PUBLIC SDL3::SDL3 SDL3_image::SDL3_image-static SDL3_mixer::SDL3_mixer-static)
    if(${WIN32})
        target_link_libraries(CWG PUBLIC SDL3main)
    endif()

    target_precompile_headers(CWG PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:<CWGPCH.hpp$<ANGLE-R>>")
    target_include_directories(CWG PUBLIC Source/Include)
    target_compile_definitions(CWG PUBLIC _USE_MATH_DEFINES)
#
