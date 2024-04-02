#include <iostream>
#include <cassert>
#include <cmath>
#include <sapling/newick.h>
#include <sapling/tree.h>
#include <sapling/phylo_node.h>

void test_visit_tree()
{
    std::string newick = "((A:0.05,B:0.1):0.15,(C:0.2,D:0.25):0.3):0.35;";
    auto tree = sapling::io::parse_newick(newick);

    std::vector<double> total_lengths = { 0.05, 0.1, 0.3, 0.2, 0.25, 0.75, 1.4 };
    size_t i = 0;

    /// can't start visiting nullptr
    /// REQUIRE_THROWS(sapling::visit_subtree(nullptr));

    /// Here we also test non-const iteration
    for (auto& node : tree)
    {
        /// run DFS from a node, calculating the total subtree branch length
        double total_length = 0.0;
        for (const auto& subtree_node : sapling::visit_subtree(&node))
        {
            total_length += subtree_node.get_branch_length();
        }

        assert(fabs(total_length - total_lengths[i]) < 1e-5);

        ++i;
    }
}

void test_postorder()
{
    std::string newick = "((A:0.05,B:0.1):0.15,(C:0.2,D:0.25):0.3):0.35;";

    const auto tree = sapling::io::parse_newick(newick);

    int iteration_count = 0;
    /// test phylo_tree::get_post_order
    for (const auto& node : tree)
    {
        // make sure this node has got the right post-order id
        assert(node.get_postorder_id() == iteration_count);

        /// find the same node in the tree by their post-order id
        const auto postorder_id = node.get_postorder_id();
        const auto found = tree.get_by_postorder_id(postorder_id);

        // make sure we found it
        assert(found);

        const auto& node_found = *found;
        // make sure it's not a null pointer
        assert(node_found);

        // compare fields
        assert(node_found->get_label() == node.get_label());
        assert(node_found->get_postorder_id() == node.get_postorder_id());
        assert(node_found->get_preorder_id() == node.get_preorder_id());
        assert(node_found->get_children() == node.get_children());
        assert(node_found->get_branch_length() == node.get_branch_length());

        ++iteration_count;
    }
}

void test_lca()
{
    std::string newick = "(((A:0.1,B:0.2)N1:0.3,(C:0.4,D:0.5)N2:0.6)N3:0.7,(E:0.8,F:0.9,G:1.0)N4:1.1)Root:1.2;";
    const auto tree = sapling::io::parse_newick(newick);

    /// Test depth node assignment
    assert((*tree.get_by_label("A"))->get_depth() == 3);
    assert((*tree.get_by_label("B"))->get_depth() == 3);
    assert((*tree.get_by_label("C"))->get_depth()  == 3);
    assert((*tree.get_by_label("D"))->get_depth()  == 3);
    assert((*tree.get_by_label("E"))->get_depth()  == 2);
    assert((*tree.get_by_label("F"))->get_depth()  == 2);
    assert((*tree.get_by_label("G"))->get_depth()  == 2);
    assert(tree.get_root()->get_depth()  == 0);

    // Test LCA for two nodes
    auto a_node = tree.get_by_label("A");
    auto d_node = tree.get_by_label("D");
    if (a_node && d_node) {
        auto lca_node = tree.lca(*a_node, *d_node);
        assert(lca_node != nullptr);
        assert(lca_node->get_label() == "N3");
    }

    // Test LCA for multiple nodes
    auto e_node = tree.get_by_label("E");
    auto f_node = tree.get_by_label("F");
    auto g_node = tree.get_by_label("G");
    if (e_node && f_node && g_node) {
        std::vector<const sapling::phylo_node*> nodes = { *e_node, *f_node, *g_node };
        auto lca_node = tree.lca(nodes);
        assert(lca_node != nullptr);
        assert(lca_node->get_label() == "N4");
    }

    auto c_node = tree.get_by_label("C");
    if (a_node && c_node && d_node) {
        std::vector<const sapling::phylo_node*> nodes = { *a_node, *c_node, *d_node };
        auto lca_node = tree.lca(nodes);
        assert(lca_node != nullptr);
        assert(lca_node->get_label() == "N3");
    }

    if (a_node && c_node && e_node) {
        std::vector<const sapling::phylo_node*> nodes = { *a_node, *c_node, *e_node };
        auto lca_node = tree.lca(nodes);
        assert(lca_node != nullptr);
        assert(lca_node->get_label() == "Root");
    }

}

int main()
{
    test_postorder();
    test_visit_tree();
    test_lca();

    std::cout << "OK" << std::endl;
    return 0;
}
