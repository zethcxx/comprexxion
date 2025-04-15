#include "tree.hpp"
#include <memory>
#include <print>
#include <stack>

bool DirTree::add_child( const std::string &name, bool is_directory ) {
    if ( curr_node->childrens.contains(name) ) {
        return false;
    }

    curr_node->childrens.insert({
        name,
        std::make_unique<Node>(name, is_directory, curr_node)
    });

    return true;
}


void DirTree::go_to_parent() {
    if ( not curr_node->parent )
        return;

    curr_node = curr_node->parent;
}


bool DirTree::go_to_child( const std::string& name ) {
    auto node = curr_node->childrens.find(name);

    if ( node == curr_node->childrens.end() )
        return false;

    curr_node = node->second.get();
    return true;
}


void DirTree::print_tree( bool indent_before, size_t indent_size ) {
    struct DirFrame {
        decltype( Node::childrens )::iterator begin;
        decltype( Node::childrens )::iterator end  ;
        size_t level;
    };

    std::stack< DirFrame > stack;

    stack.push({
        .begin = root->childrens.begin(),
        .end   = root->childrens.end  (),
        .level = 1
    });

    if ( indent_before )
        std::print("{:{}}", " ", indent_size);

    std::println("\x1b[34m{}\x1b[0m", root->name);
    bool is_last_element = false;

    while ( not stack.empty()) {
        auto &[it, it_end, level] = stack.top();

        if ( it == it_end ) {
            stack.pop();
            continue;
        }

        const auto &node = it->second;

        if ( indent_before )
            std::print("{:{}}", " ", indent_size);

        it++;

        if ( level == 1 && it == it_end )
            is_last_element = true;

        if (level != 1 ) {
            for (size_t i = 1 ; i < level; i++)
                std::print("\x1b[30m{}\x1b[0m", is_last_element ? "    " : "│   ");
        }

        std::print("\x1b[30m{}\x1b[0m", ( it == it_end ? "└── " : "├── " ));


        if ( node->is_directory )
            std::print("\x1b[34m");

        std::println("{}\x1b[0m", node->name);

        if ( node->is_directory and not node->childrens.empty()) {
            stack.push({
                .begin = node->childrens.begin(),
                .end   = node->childrens.end  (),
                .level = level + 1
            });
        }
    }
}

DirTree::Node::Node ( const std::string &_name, bool _is_directory, Node *_parent )
  : name         { _name         },
    is_directory { _is_directory },
    parent       { _parent       }
{}


DirTree::DirTree ()
  : root      { std::make_unique<Node>(".", true) },
    curr_node { root.get()                        }
{}
