cmake_minimum_required(VERSION 3.14)
project(sapling)

set(CMAKE_CXX_STANDARD 17)

# Find required Boost components
find_package(Boost COMPONENTS iostreams filesystem REQUIRED)

# Define the library target
add_library(sapling
        src/tree.cpp include/sapling/tree.h
        src/phylo_node.cpp include/sapling/phylo_node.h
        src/newick.cpp include/sapling/newick.h
        src/io.cpp include/sapling/io.h
        include/sapling/sapling.h)

# Include directories for the library
target_include_directories(sapling PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

# Link libraries
target_link_libraries(sapling PUBLIC Boost::iostreams Boost::filesystem)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DSAPLING_DEBUG)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address)
    link_libraries(-fsanitize=address)
endif()



# Option to build tests
option(BUILD_TESTS "Build the tests for Sapling" OFF)

# Enable fPIC
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(TARGET sapling PROPERTY POSITION_INDEPENDENT_CODE ON)


# Test target
if(BUILD_TESTS)
    SET(CMAKE_BUILD_TYPE "Debug")
    add_executable(tests tests/tests.cpp)
    target_include_directories(tests PRIVATE include)
    target_link_libraries(tests PRIVATE sapling)
endif()


# Installation rules for the library
install(TARGETS sapling
        EXPORT sapling-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

# Install the header files
install(DIRECTORY include/sapling DESTINATION include)

# Generate and install the export file
install(EXPORT sapling-targets
        FILE saplingTargets.cmake
        NAMESPACE sapling::
        DESTINATION lib/cmake/sapling)

# Create a version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file("saplingConfigVersion.cmake"
        VERSION 0.1
        COMPATIBILITY SameMajorVersion)

# Install the config and version files
install(FILES "saplingConfig.cmake" "saplingConfigVersion.cmake"
        DESTINATION lib/cmake/sapling)