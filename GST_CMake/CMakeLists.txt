cmake_minimum_required(VERSION 3.10)

project(GST-DeckLink)

# Determine OS bitness
set(OSBitness 32)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(OSBitness 64)
endif()

# Set output directories based on OS and build type
set(FullOutputDir "${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}${OSBitness}/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${FullOutputDir}/static_libs")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${FullOutputDir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${FullOutputDir}")

# Find required packages - GStreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-video-1.0)

# Create the executable
add_executable(
    ${PROJECT_NAME}
    main.c
)

# Configure include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${GST_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${GST_LIBRARIES})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -g ${GST_CFLAGS})