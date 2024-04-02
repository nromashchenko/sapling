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


#include <sapling/tree.h>
#include <sapling/newick.h>
#include <filesystem>
#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;
using namespace sapling;
using namespace sapling::impl;
using std::vector;
using std::string;
using std::move;
using std::begin, std::end;


tree::tree(phylo_node* root)
    : _root{ root }, _node_count{ 0 }
{
    index();
}

tree::tree(tree&& other) noexcept
{
    _root = other._root;
    other._root = nullptr;

    _node_count = other._node_count;
    other._node_count = 0;

    _preorder_id_to_node = std::move(other._preorder_id_to_node);
    _postorder_id_node_mapping = std::move(other._postorder_id_node_mapping);
    _label_to_node = std::move(other._label_to_node);
}

tree::~tree() noexcept
{
    delete _root;
}

tree::const_iterator tree::begin() const noexcept
{
    return visit_subtree(_root).begin();
}

tree::const_iterator tree::end() const noexcept
{
    return visit_subtree(_root).end();
}

tree::iterator tree::begin() noexcept
{
    return visit_subtree<postorder_tree_iterator<false>>(_root).begin();
}

tree::iterator tree::end() noexcept
{
    return visit_subtree<postorder_tree_iterator<false>>(_root).end();
}

size_t tree::get_node_count() const noexcept
{
    return _node_count;
}

tree::value_pointer tree::get_root() const noexcept
{
    return _root;
}

void tree::set_root(value_pointer root)
{
    _root = root;
}

bool tree::is_rooted() const noexcept
{
    return _root && _root->get_children().size() < 3;
}

void tree::index()
{
    if (_root->get_parent())
    {
        throw std::invalid_argument{ "Can not create a tree from non-root node: "
                                     "the parent of the root must be nullptr."};
    }

    /// Recreate all search maps
    _index_preorder_id();
    _index_postorder_id();
    _index_labels();
    _index_depth();
    _index_nodes();
}

std::optional<const phylo_node*> tree::get_by_preorder_id(phylo_node::id_type preorder_id) const noexcept
{
    if (const auto it = _preorder_id_to_node.find(preorder_id); it != _postorder_id_node_mapping.end())
    {
        return { it->second };
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<const phylo_node*> tree::get_by_postorder_id(phylo_node::id_type postorder_id) const noexcept
{
    if (const auto it = _postorder_id_node_mapping.find(postorder_id); it != _postorder_id_node_mapping.end())
    {
        return { it->second };
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<const phylo_node*> tree::get_by_label(const std::string& label) const noexcept
{
    if (const auto it = _label_to_node.find(label); it != _label_to_node.end())
    {
        return { it->second };
    }
    else
    {
        return std::nullopt;
    }
}

tree tree::copy() const
{
    auto new_root = _root->copy();
    return tree(new_root);
}


const phylo_node* tree::lca(const phylo_node* x, const phylo_node* y) const
{
    /// Step 1: Bring x and y to the same level
    while (x->get_depth() > y->get_depth()) {
        x = x->get_parent();
    }
    while (y->get_depth() > x->get_depth()) {
        y = y->get_parent();
    }

    /// Step 2: Find common ancestor
    while (x != y) {
        x = x->get_parent();
        y = y->get_parent();
    }

    return x;
}

const phylo_node* tree::lca(const std::vector<const phylo_node*>& nodes) const
{
    if (nodes.empty())
    {
        return nullptr;
    }

    const phylo_node* current_lca = nodes[0];
    for (size_t i = 1; i < nodes.size(); ++i) {
        current_lca = lca(current_lca, nodes[i]);
    }
    return current_lca;
}

void tree::_index_preorder_id()
{
    _preorder_id_to_node.clear();

    phylo_node::id_type preorder_id = 0;
    for (auto& node : visit_subtree<preorder_tree_iterator<false>>(_root))
    {
        node._preorder_id = preorder_id;
        _preorder_id_to_node[preorder_id] = &node;
        ++preorder_id;
    }
}

void tree::_index_postorder_id()
{
    _postorder_id_node_mapping.clear();

    phylo_node::id_type postorder_id = 0;
    for (auto& node : visit_subtree<postorder_tree_iterator<false>>(_root))
    {
        node._postorder_id = postorder_id;
        _postorder_id_node_mapping[postorder_id] = &node;
        ++postorder_id;
    }
}

void tree::_index_labels()
{
    _label_to_node.clear();

    for (auto& node : visit_subtree(_root))
    {
        _label_to_node[node.get_label()] = &node;
    }
}

void tree::_index_depth()
{
    _index_depth_recursive(_root, 0);
}

void tree::_index_depth_recursive(phylo_node* node, int depth)
{
    if (!node)
    {
        return;
    }

    node->set_depth(depth);

    for (phylo_node* child : node->get_children())
    {
        _index_depth_recursive(child, depth + 1);
    }
}

void tree::_index_nodes()
{
    _node_count = 0;

    for (auto& node : visit_subtree<iterator>(_root))
    {
        (void)node;
        ++_node_count;

        if (node.is_leaf())
        {
            /// By convention the leaves have no nodes in their subtrees
            node.set_num_nodes(0);

            /// By convention the number of leaves in the subtree of a leaf is one
            node.set_num_leaves(1);

            /// There is no subtree
            node.set_subtree_branch_length(0.0);
        }
        else
        {
            /// The number of the nodes in the subtree of this node
            ///   =  the number of children
            size_t total_num_nodes = node.get_children().size();
            size_t total_num_leaves = 0;

            /// The total branch length in the subtree
            phylo_node::branch_length_type subtree_branch_length = 0.0;

            for (const auto& child : node.get_children())
            {
                ///  + the total number of nodes in subtrees of children
                total_num_nodes += child->get_num_nodes();

                /// The number of leaves is the sum of leaves of all children
                total_num_leaves += child->get_num_leaves();

                subtree_branch_length += child->get_subtree_branch_length() + child->get_branch_length();
            }

            node.set_num_nodes(total_num_nodes);
            node.set_num_leaves(total_num_leaves);
            node.set_subtree_branch_length(subtree_branch_length);
        }
    }

    // Assuming other necessary tree and node functions are defined
}


namespace sapling
{
    void save_tree(const tree& tree, const std::string& filename)
    {
        std::ofstream out(filename);
        out << sapling::io::to_newick(tree);
    }
}