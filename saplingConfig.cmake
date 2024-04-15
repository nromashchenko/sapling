# Assume the project exports its targets with the namespace "sapling::"
include(CMakeFindDependencyMacro)
find_dependency(Boost COMPONENTS iostreams filesystem REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/saplingTargets.cmake")