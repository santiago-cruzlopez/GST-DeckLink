cmake_minimum_required(VERSION 3.5)
project(DeckLink-SDK)

set(OSBitness 32)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(OSBitness 64)
endif()

set(FullOutputDir "${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}${OSBitness}/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${FullOutputDir}/static_libs")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${FullOutputDir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${FullOutputDir}")

# Define DeckLink SDK path
set(DECKLINK_SDK_PATH "/home/{username}/Blackmagic_DeckLink_SDK_12.8/Linux" CACHE PATH "Path to DeckLink SDK")

# DeckLink SDK source files
set(DECKLINK_SOURCES
    "${DECKLINK_SDK_PATH}/include/DeckLinkAPIDispatch.cpp"
)

set(APP_SOURCES
    "${CMAKE_SOURCE_DIR}/src/callbacks.cpp"
    "${CMAKE_SOURCE_DIR}/src/decklink_utils.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} 
    main.cpp
    ${DECKLINK_SOURCES}
    ${APP_SOURCES}
)

# Configure include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    "${DECKLINK_SDK_PATH}/include"
    "${CMAKE_SOURCE_DIR}/src"
)

# Link libraries
target_link_libraries(${PROJECT_NAME})

# Platform-specific configurations for DeckLink
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif()