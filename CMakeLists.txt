cmake_minimum_required(VERSION 2.8)

# Set variables
set(PROJECT_NAME RobotArm2_Take_Mahjong)


# Environment variables
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation directory")
set(BUILD_TEST OFF CACHE BOOL "Enable building test")

# Set project
project(${PROJECT_NAME} ${PROJECT_LANGUAGE})

include(${CMAKE_CURRENT_SOURCE_DIR}/deps/deps.cmake)

# Include subdirectory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(${BUILD_TEST})
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()

# Include utility cmake setting
include (${CMAKE_CURRENT_SOURCE_DIR}/util/util.cmake)
