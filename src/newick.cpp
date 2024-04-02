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



#include <iostream>
#include <stack>
#include <boost/algorithm/string/predicate.hpp>
//#include <boost/tokenizer.hpp>
#include <iomanip>
#include <sstream>
#include <sapling/io.h>
#include <sapling/tree.h>
#include <sapling/newick.h>

using std::string, std::string_view;
using namespace sapling;
using namespace sapling::io;

namespace sapling::io
{
    /// \brief A class for parsing .newick-formatted files.
    /// \details This class parses phylogenetic trees in the newick format. It designed to support
    ///  a buffered reading from disk.
    class newick_parser
    {
    public:
        newick_parser();
        newick_parser(const newick_parser&) = delete;
        newick_parser(newick_parser&&) = delete;
        ~newick_parser() = default;

        /// \brief Parses an input buffer. This function can be called more than once,
        /// during the buffered reading from disk.
        /// \param data A string variable containing the current buffer data to parse.
        void parse(std::string_view data);

        phylo_node* get_root() const;

    private:
        /// \brief Parses next symbol of input data.
        /// \param ch A character to parse
        void _parse_character(char ch);

        /// \brief Handles the quote character
        void _handle_quote();

        /// \brief Handles a left parenthesis in input data.
        /// \details A left parenthesis indicates that a new node with children should be created.
        /// We will create it later though, during the _handle_right_parenthesis call
        /// because the phylo_node class has no default constructor for some design reasons.
        /// \sa phylo_node::phylo_node, _handle_right_parenthesis
        void _handle_left_parenthesis();

        /// \details The list of children for "current" parent node is over.
        /// The next symbols are referred to the parent node
        void _handle_right_parenthesis();

        /// \details Node delimiter, we create a node from the text content we collected so far
        void _handle_comma();

        /// \details End of file, we take last node as root
        void _handle_semicolon();

        /// \details Keep reading the current node description
        /// \param ch A character to parse
        void _handle_text(char ch);

        void _start_node();
        phylo_node* _finish_node();
        void _parse_node_text();

        std::stack<phylo_node*> _node_stack;
        phylo_node* _root;

        std::string _node_text;

        bool _parsing_node;
        bool _end_of_file;
        bool _parsing_text;
    };
}

newick_parser::newick_parser()
    : _root{ nullptr }
      , _parsing_node{ false }
      , _end_of_file{ false }
      , _parsing_text{ false }
{}

void newick_parser::parse(string_view data)
{
    for (char c : data)
    {
        _parse_character(c);

        if (_end_of_file)
        {
            break;
        }
    }
}

phylo_node* newick_parser::get_root() const
{
    return _root;
}

void newick_parser::_parse_character(char ch)
{
    if (_parsing_text && ch != '\'')
    {
        _handle_text(ch);
    }
    else
    {
        switch (ch)
        {
            case '\'':
                _handle_quote();
                break;
            case '(':
                _handle_left_parenthesis();
                break;
            case ')':
                _handle_right_parenthesis();
                break;
            case ',':
                _handle_comma();
                break;
            case ';':
                _handle_semicolon();
                break;
            default:
                _handle_text(ch);
                break;
        }
    }
}

void newick_parser::_handle_quote()
{
    _parsing_text = !_parsing_text;
}

void newick_parser::_handle_left_parenthesis()
{
    _start_node();
    _parsing_node = false;
}

void newick_parser::_handle_right_parenthesis()
{
    _finish_node();
    _parsing_node = true;
}

void newick_parser::_handle_comma()
{
    _finish_node();
}

void newick_parser::_handle_semicolon()
{
    _root = _finish_node();
    _end_of_file = true;
}

void newick_parser::_handle_text(char ch)
{
    /// A node can start from a parenthesis (ex. "(A, B)C") or from a label (ex. "A").
    /// The second case is equal to "()A". In this case we need to create a node as soon
    /// as we read the first symbol
    if (!_parsing_node)
    {
        _start_node();
        _parsing_node = true;
    }

    /// Keep reading the node description
    _node_text.push_back(ch);
}

void newick_parser::_start_node()
{
    phylo_node* parent = _node_stack.empty() ? nullptr : _node_stack.top();
    _node_stack.push(new phylo_node());
    _node_stack.top()->set_parent(parent);
}

phylo_node* newick_parser::_finish_node()
{
    _parse_node_text();

    /// Add the node to its parent's list
    phylo_node* current_node = _node_stack.top();
    _node_stack.pop();

    /// Add the node to its parent, if exists
    if (current_node->get_parent() != nullptr)
    {
        current_node->get_parent()->add_child(current_node);
    }

    _parsing_node = false;
    return current_node;
}

void newick_parser::_parse_node_text()
{
    phylo_node* current_node = _node_stack.top();

    // the content can be like "node_label:branch_length", ":branch_length", "node_label" or just ""
    if (!_node_text.empty())
    {
        const auto found = _node_text.find_last_of(':');

        // if node label presented
        if (!boost::starts_with(_node_text, ":"))
        {
            current_node->set_label(_node_text.substr(0, found));
        }

        if (found != std::string::npos)
        {
            const auto len_str = _node_text.substr(found + 1);
            current_node->set_branch_length(std::stof(len_str));
        }
    }

    // the current node is over
    _node_text.clear();
}

tree sapling::io::load_newick(const string& file_name)
{
    std::cout << "Loading newick: " + file_name << std::endl;

    /// Load a tree from file
    newick_parser parser;
    sapling::io::buffered_reader reader(file_name);
    if (reader.good())
    {
        while (!reader.empty())
        {
            auto chunk = reader.read_next_chunk();
            parser.parse(chunk);
        }
    }
    else
    {
        throw std::runtime_error("Cannot open file: " + file_name);
    }

    /// Assign post-order ids to the phylo_node's
    auto tree = sapling::tree{parser.get_root() };
    std::cout << "Loaded a tree of " << tree.get_node_count() << " nodes.\n\n" << std::flush;
    return tree;
}

tree sapling::io::parse_newick(std::string_view newick_string)
{
    newick_parser parser;
    parser.parse(newick_string);
    return tree{parser.get_root() };
}


void to_newick(std::ostream& out, const phylo_node& node, bool jplace)
{
    const auto num_children = node.get_children().size();
    if (num_children > 0)
    {
        out << "(";
        size_t i = 0;
        for (; i < num_children - 1; ++i)
        {
            to_newick(out, *node.get_children()[i], jplace);
            out << ",";
        }
        to_newick(out, *node.get_children()[num_children - 1], jplace);
        out << ")";
    }

    if (!node.get_label().empty())
    {
        out << node.get_label();
    }

    out << ":" << std::setprecision(10) << node.get_branch_length();
    if (jplace)
    {
        out << "{" << node.get_postorder_id() << "}";
    }
}

std::string sapling::io::to_newick(const tree& tree, bool jplace)
{
    std::ostringstream stream;
    ::to_newick(stream, *(tree.get_root()), jplace);
    stream << ";";
    return stream.str();
}