// MIT License
//
// Copyright (c) 2024 Nikolai Romashchenko
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
//     of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
//     to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//     copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
//     copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//     AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#ifndef SAPLING_NEWICK_H
#define SAPLING_NEWICK_H

#include <string>
#include <string_view>

namespace sap
{
    class phylo_tree;
}

namespace sap::io
{
    /// \brief Loads a phylogenetic tree from a newick formatted file.
    sap::phylo_tree load_newick(const std::string& file_name);

    /// \brief Parses a phylogenetic tree from a newick formatted string.
    sap::phylo_tree parse_newick(std::string_view newick_string);

    /// \brief Constructs a Newick-formatted string from the input tree.
    /// Depending on the jplace parameter, it builds
    ///     false) Pure newick: (label:branch_length,label:branch_length)...
    ///     true) Jplace: (label:branch_length{node_postorder_id}, ...)...
    std::string to_newick(const sap::phylo_tree& tree, bool jplace=false);
}

/// \brief Outputs a tree in Jplace-extended Newick format:
/// (label:branch_length{node_postorder_id}, ...)
std::ostream& operator<<(std::ostream& out, const sap::phylo_tree& tree);


#endif //SAPLING_NEWICK_H
