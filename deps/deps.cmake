# Set dependences paths

# Find other dependences
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_path(SERIALPORT_INCLUDE_DIR libserialport.h
	"/usr/include"
	"/usr/local/include"
	)
find_library(SERIALPORT_LIB libserialport.a
	"/usr/lib"
	"/usr/local/lib"
	)

find_path(ANN_INCLUDE_DIR ann.h
	"/usr/include"
	"/usr/local/include"
	)
find_library(ANN_LIB libann.a
	"/usr/lib"
	"/usr/local/lib"
	)

include_directories(${SERIALPORT_INCLUDE_DIR})
include_directories(${ANN_INCLUDE_DIR})

set(DEPS_PATHS
	${CMAKE_CURRENT_SOURCE_DIR}/deps/mahjong_model
	${CMAKE_CURRENT_SOURCE_DIR}/deps/Retinex
	)
include_directories(${DEPS_PATHS})

# Add subdirectory
foreach(DEPS_PATH ${DEPS_PATHS})
	add_subdirectory(${DEPS_PATH})
endforeach()
