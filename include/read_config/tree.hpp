#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class DirTree {

public:

    // --- Constructor
    DirTree();


    // --- Tree Printing
    void print_tree  ( size_t initial_indent = 0);


    // --- Current Node Methods
    [[nodiscard]]
    bool go_to_parent( void );
    // +
    bool has_parent  ( void ) const;
    // +
    bool go_to_child ( const std::string& name );
    // +
    [[nodiscard]]
    bool add_child( const std::string& name,
                    const bool is_directory = true );


    // --- Helper Method:
    [[nodiscard]]
    bool has_errors( void ) const {
        return _has_errors;
    }


private:

    struct Node {
        // --- Basic Properties:
        std::string name;
        bool is_directory;
        Node *parent;

        // --- Children Management:
        std::unordered_map<
            std::string /* name of child */,
            std::unique_ptr<Node>> children {};

        // --- Constructor:
        Node (
            const std::string &_name,
            const bool _is_directory,
            Node *_parent = nullptr
        );
    };


    // --- Root Node Of Tree:
    std::unique_ptr<Node> root;

    // --- Current Node Of Tree:
    Node* curr_node;


    // --- Error State:
    bool _has_errors = false;
};
