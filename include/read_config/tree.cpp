#include "tree.hpp"
#include <memory>
#include <print>
#include <vector>
#include <utility>

bool DirTree::has_parent() const {
    return curr_node->parent != nullptr;

}

bool DirTree::add_child( const std::string &name,
                         const bool is_directory
) {
    const auto &children = curr_node->children;

    if ( not curr_node->is_directory or children.contains(name) )
        return false;

    curr_node->children.insert({
        name,
        std::make_unique<Node>(name, is_directory, curr_node)
    });

    return true;
}


bool DirTree::go_to_parent() {
    // --- if curr_node is root
    if ( not has_parent() )
        return false;

    curr_node = curr_node->parent;
    return true;
}


bool DirTree::go_to_child( const std::string& name ) {
    const auto &node = curr_node->children.find(name);

    if ( node == curr_node->children.end() )
        return false;

    curr_node = node->second.get();
    return true;
}


void DirTree::print_tree( size_t initial_indent ) {
    static std::string indent_str = std::string(initial_indent, ' ');

    using std::print, std::println;

    struct DirFrame {
        decltype ( Node::children )::iterator begin;
        decltype ( Node::children )::iterator end  ;
        bool separator;
    };

    std::vector<DirFrame> stack;

    stack.push_back({
        .begin = root->children.begin(),
        .end   = root->children.end  (),
        .separator = true,
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
            if ( stack.at( i ).separator )
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

        if ( node->is_directory )
            println("\x1b[34m{}\x1b[0m", name);
        else
            println("{}", name);

        it++;

        if ( not node->is_directory and node->children.empty())
            continue;

        stack.push_back({
            .begin = node->children.begin(),
            .end   = node->children.end  (),
            .separator = true
        });
    }
}


DirTree::Node::Node (
    const std::string &_name,
    const bool _is_directory,
    Node *_parent
)
  : name         { _name         },
    is_directory { _is_directory },
    parent       { _parent       }
{}


DirTree::DirTree ()
  : root      { std::make_unique<Node>("./", true) },
    curr_node { root.get()                         }
{}
