cmake_minimum_required(VERSION 3.27)
project(sapling)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost COMPONENTS iostreams filesystem REQUIRED)

add_executable(sapling
        src/main.cpp
        src/phylo_tree.cpp include/sapling/tree.h
        src/phylo_node.cpp include/sapling/phylo_node.h
        src/newick.cpp include/sapling/newick.h
        src/io.cpp include/sapling/io.h
        include/sapling/phylo_node.h)

target_include_directories(sapling PUBLIC include)
target_link_libraries(sapling PUBLIC Boost::iostreams Boost::filesystem)