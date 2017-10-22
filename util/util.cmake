# Set paths
set(UTIL_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/util/ann_training)

# Add subdirectories
foreach(UTIL_PATH ${UTIL_PATHS})
	add_subdirectory(${UTIL_PATH})
endforeach()
