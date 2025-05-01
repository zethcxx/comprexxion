// ---- LOCAL INCLUDES ----
//
#include "parsing/tree.hpp"


// ---- EXTERNAL INCLUDES ----
//
#include <fmt/core.h>


// ---- STANDARD INCLUDES ----
//
#include <memory>
#include <vector>
#include <utility>
#include <filesystem>
#include <stack>


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
    /* if curr_node is root */
    if ( not has_parent() )
        return Errors::NO_SUCH_PARENT;

    curr_node = curr_node->parent;
    return Errors::NONE;
}


void DirTree::ascend_levels( size_t levels ) {
    while ( levels --> 0 and has_parent() )
        (void)go_to_parent();
}


DirTree::Errors DirTree::go_to_child( const std::string& name ) {
    const auto &node = curr_node->children.find(name);

    if ( node == curr_node->children.end() )
        return Errors::NO_SUCH_CHILD;


    curr_node = node->second.get();
    return Errors::NONE;
}


DirTree::Errors DirTree::select_all_of( const Node& node ) {
    namespace fs = std::filesystem;

    if ( not node.is_directory() )
        return Errors::INVALID_TYPE;

    if ( not fs::exists( node.name ))
        return Errors::INVALID_PATH;


    const auto firstIterator = fs::directory_iterator( node.name );


    std::stack<fs::directory_iterator> stack {{
        firstIterator
    }};


    while ( not stack.empty() ) {
        auto &dirEntry = stack.top();

        if ( dirEntry == fs::directory_iterator{} ) {
            stack.pop();
            (void)go_to_parent();
            continue;
        }


        const auto path_name = dirEntry->path().filename().string();
        const auto type_path = dirEntry->is_directory()
            ? NodeType::IS_DIRECTORY
            : NodeType::IS_FILE;


        if ( add_child( path_name, type_path )  != Errors::NONE )
            continue;


        if ( dirEntry->is_directory() ) {
            go_to_child( path_name );
            stack.push( fs::directory_iterator( dirEntry->path() ) );
        }

        dirEntry++;
    }

    go_to_child( node.name );

    return Errors::NONE;
}


void DirTree::print_tree( size_t initial_indent ) const noexcept {
    static std::string indent_str = std::string(initial_indent, ' ');

    using fmt::print, fmt::println;

    struct DirFrame {
        children_node_t::const_iterator begin;
        children_node_t::const_iterator end  ;
        bool sep;
    };

    std::vector<DirFrame> stack;

    stack.push_back(
        DirFrame {
            .begin = get_root().children.begin(),
            .end   = get_root().children.end  (),
            .sep   = true,
        }
    );


    print("{}", indent_str);
    println("root(\x1b[34m{}\x1b[0m)", get_root().name);


    while ( not stack.empty() ) {
        auto &[it, it_end, sep] = stack.back();

        if ( it == it_end ) {
            stack.pop_back();
            continue;
        }

        const auto &name    =  ( it -> first );
        const auto &node    = *( it -> second);
        const auto &next_it = std::next ( it );

        print("{}", indent_str);

        for ( size_t i = 0; i < stack.size()-1; i++ ) {
            if ( stack.at( i ).sep )
                print("\x1b[90m│  \x1b[0m");
            else
                print("   ");
        }


        if ( next_it == it_end ) {
            sep = false;
            print("\x1b[90m└── \x1b[0m");

        } else print("\x1b[90m├── \x1b[0m");


        if ( node.is_directory() )
            println("\x1b[34m{}\x1b[0m", name);
        else
            println("{}", name);

        it++;

        if ( not node.is_directory() and node.children.empty())
            continue;

        stack.push_back(
            DirFrame {
                .begin = node.children.begin(),
                .end   = node.children.end  (),
                .sep   = true
            }
        );

    }
}


const DirTree::Node& DirTree::get_root( void ) const {
    return *root.get();
}


const DirTree::Node& DirTree::get_curr_node( void ) const {
    return *curr_node;
}


bool DirTree::Node::is_directory( void ) const {
    return type == NodeType::IS_DIRECTORY;
}


DirTree::Node::Node (
    const std::string &_name ,
    NodeType           _type ,
    Node              *_parent
)
  : name   { _name   },
    type   { _type   },
    parent { _parent }
{}


DirTree::DirTree ()
  : root      { std::make_unique<Node>( "./", NodeType::IS_DIRECTORY ) },
    curr_node { root.get() }
{
    namespace fs = std::filesystem;

    if ( not fs::exists( root->name ) ) {
        curr_error = Errors::INVALID_PATH;
    }
}


DirTree::DirTree (const std::string &_root_name)
  : root      { std::make_unique<Node>( _root_name, NodeType::IS_DIRECTORY ) },
    curr_node { root.get() }
{}
