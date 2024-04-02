cmake_minimum_required(VERSION 3.27)
project(sapling)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost COMPONENTS iostreams filesystem REQUIRED)

add_executable(sapling
        main.cpp
        phylo_tree.cpp phylo_tree.h
        phylo_node.cpp phylo_node.h
        newick.h newick.cpp
        io.h io.cpp)

target_link_libraries(sapling PUBLIC Boost::iostreams Boost::filesystem)