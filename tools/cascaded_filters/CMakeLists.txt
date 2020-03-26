find_package(dsps REQUIRED)

# Create executable
add_executable(cascaded-filters
	${CMAKE_CURRENT_SOURCE_DIR}/main.cc
)

# Link libraries
target_link_libraries(cascaded-filters
    dsps
)

# Executable location
set_target_properties(cascaded-filters PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/"
)
