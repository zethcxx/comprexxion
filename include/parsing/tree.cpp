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
#include <filesystem>
#include <stack>


/* ----------------------- DIRTREE:: IMPLEMENTATION ----------------------- */

DirTree::Errors DirTree::add_child(
    const std::string &name,
    NodeType type
) {
    return (*curr_node).add_child( name, type );
}


DirTree::Errors DirTree::go_to_parent() {
    /* if curr_node is root */
    if ( not (*curr_node).has_parent() )
        return Errors::NO_SUCH_PARENT;

    curr_node = (*curr_node).get_parent();
    return Errors::NONE;
}


void DirTree::ascend_levels( size_t levels ) {
    while ( levels --> 0 and (*curr_node).has_parent() )
        (void)go_to_parent();
}


DirTree::Errors DirTree::go_to_child( const std::string& name ) {
    const auto &node = (*curr_node).get_children().find(name);

    if ( node == (*curr_node).get_children().end() )
        return Errors::NO_SUCH_CHILD;


    curr_node = (*node).second.get();
    return Errors::NONE;
}


DirTree::Errors DirTree::select_all_of( const Node& node ) {
    namespace fs = std::filesystem;

    if ( not node.is_directory() )
        return Errors::INVALID_TYPE;

    if ( not fs::exists( node.get_name() ))
        return Errors::INVALID_PATH;


    const auto firstIterator = fs::directory_iterator( node.get_name() );


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

    go_to_child( node.get_name() );

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
            .begin = get_root().get_children().begin(),
            .end   = get_root().get_children().end  (),
            .sep   = true,
        }
    );


    print("{}", indent_str);
    println("root(\x1b[34m{}\x1b[0m)", get_root().get_name());


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

        if ( not node.is_directory() and node.get_children().empty())
            continue;

        stack.push_back(
            DirFrame {
                .begin = node.get_children().begin(),
                .end   = node.get_children().end  (),
                .sep   = true
            }
        );

    }
}


std::filesystem::path DirTree::get_full_path_of( const Node& node ) {
    std::vector<std::string_view> path_parts;
    const Node *current_node = &node;

    while ( (*current_node).has_parent() ) {
        path_parts.push_back( (*current_node).get_name() );
        current_node = (*current_node).get_parent();
    }

    std::filesystem::path full_path;

    for ( auto it = path_parts.rbegin(); it != path_parts.rend(); it++ ) {
        full_path /= *it;
    }

    return full_path;
}


DirTree::DirTree ()
  : root      { std::make_unique<Node>( "./", NodeType::IS_DIRECTORY ) },
    curr_node { root.get() }
{
    namespace fs = std::filesystem;

    if ( not fs::exists( (*root).get_name() ) ) {
        curr_error = Errors::INVALID_PATH;
    }
}


DirTree::DirTree (const std::string &_root_name)
  : root      { std::make_unique<Node>( _root_name, NodeType::IS_DIRECTORY ) },
    curr_node { root.get() }
{}


/* -------------------- DIRTREE::NODE:: IMPLEMENTATION -------------------- */


bool DirTree::Node::is_directory( void ) const {
    return type == NodeType::IS_DIRECTORY;
}


const DirTree::Node& DirTree::get_root( void ) const {
    return *root.get();
}


const DirTree::Node& DirTree::get_curr_node( void ) const {
    return *curr_node;
}


std::string   DirTree::Node::get_full_path( void ) const {
    return get_full_path_of( *this );
}


bool DirTree::Node::has_parent( void ) const {
    return this->parent != nullptr;
}


const std::string &DirTree::Node::get_name( void ) const {
    return name;
}


const DirTree::children_node_t &DirTree::Node::get_children( void ) const {
    return children;
}


DirTree::Node *DirTree::Node::get_parent( void ) const {
    return parent;
}


DirTree::Errors DirTree::Node::add_child (
    const std::string &_name,
    NodeType _type
) {
    auto &current_node = *this;

    if ( not current_node.is_directory() )
        return Errors::EXPECTED_DIRECTORY;

    else if ( current_node.children.contains(name) )
        return Errors::ALREADY_EXISTS;


    current_node.children.insert( {
        _name,
        std::make_unique<Node>(
            _name,
            _type,
            &current_node
        )
    });

    return Errors::NONE;
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
