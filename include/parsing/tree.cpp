// --- My Includes:
#include "parsing/tree.hpp"

// --- External Includes:
#include <fmt/core.h>

// --- Standard Includes:
#include <memory>
#include <vector>
#include <utility>


bool DirTree::has_parent() const {
    return curr_node->parent != nullptr;
}


DirTree::Errors DirTree::add_child(
    const std::string &name,
    NodeType type
) {
    const auto &parent   = curr_node;
    const auto &children = curr_node->children;

    if ( not parent->is_directory() )
        return Errors::INVALID_TYPE;

    else if ( children.contains(name) )
        return Errors::ALREADY_EXISTS;


    parent->children.insert({
        name,
        std::make_unique<Node>( name, type, parent )
    });

    return Errors::NONE;
}


DirTree::Errors DirTree::go_to_parent() {
    // --- if curr_node is root
    if ( not has_parent() )
        return Errors::NO_SUCH_PARENT;

    curr_node = curr_node->parent;
    return Errors::NONE;
}


DirTree::Errors DirTree::go_to_child( const std::string& name ) {
    const auto &node = curr_node->children.find(name);

    if ( node == curr_node->children.end() )
        return Errors::NO_SUCH_CHILD;


    curr_node = node->second.get();
    return Errors::NONE;
}


void DirTree::print_tree( size_t initial_indent ) const noexcept {
    static std::string indent_str = std::string(initial_indent, ' ');

    using fmt::print, fmt::println;

    struct DirFrame {
        children_node_t::iterator begin;
        children_node_t::iterator end  ;
        bool sep;
    };

    std::vector<DirFrame> stack;

    stack.push_back({
        .begin = root->children.begin(),
        .end   = root->children.end  (),
        .sep   = true,
    });

    print("{}", indent_str);
    println("root(\x1b[34m{}\x1b[0m)", root->name);

    while ( not stack.empty() ) {
        auto &[it, it_end, separator] = stack.back();

        if ( it == it_end ) {
            stack.pop_back();
            continue;
        }

        const auto &name    = it -> first  ;
        const auto &node    = it -> second ;
        const auto &next_it = std::next(it);

        print("{}", indent_str);

        for ( size_t i = 0; i < stack.size()-1; i++ ) {
            if ( stack.at( i ).sep )
                print("│  ");
            else
                print("   ");
        }

        if ( next_it == it_end ) {
            separator = false;
            print("└── ");
        } else {
            print("├── ");
        }

        if ( node->is_directory() )
            println("\x1b[34m{}\x1b[0m", name);
        else
            println("{}", name);

        it++;

        if ( not node->is_directory() and node->children.empty())
            continue;

        stack.push_back({
            .begin = node->children.begin(),
            .end   = node->children.end  (),
            .sep = true
        });
    }
}


const DirTree::Node* DirTree::get_root( void ) const {
    return root.get();
}


const DirTree::Node* DirTree::get_curr_node( void ) const {
    return curr_node;
}


// --- Node Constructor:
DirTree::Node::Node (
    const std::string &_name,
    NodeType _type,
    Node *_parent
)
  : name   { _name   },
    type   { _type   },
    parent { _parent }
{}


// --- DirTree Default Constructor:
DirTree::DirTree ()
  : root      { std::make_unique<Node>( "./", NodeType::IS_DIRECTORY ) },
    curr_node { root.get() }
{}

// --- DirTree Custom Constructor:
DirTree::DirTree (const std::string &_root_name)
  : root      { std::make_unique<Node>( _root_name, NodeType::IS_DIRECTORY ) },
    curr_node { root.get() }
{}
