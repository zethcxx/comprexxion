#pragma once

// --- Standard Includes:
#include <memory>
#include <string>
#include <unordered_map>


class DirTree {

public:

    // --- Constructors:
    DirTree();
    DirTree( const std::string& _root_name );


    // --- Errors Types:
    enum class Errors : std::uint8_t {
        NONE,
        NO_SUCH_CHILD,
        NO_SUCH_PARENT,
        ALREADY_EXISTS,
        INVALID_NAME,
        INVALID_PATH,
        INVALID_TYPE,
        INVALID_VALUE
    };


    // --- Tree Printing
    void print_tree ( size_t initial_indent = 0 ) const noexcept;


    // --- Current Node Methods
    [[nodiscard]]
    Errors go_to_parent( void );
    Errors go_to_child ( const std::string& name );
    Errors add_child   ( const std::string& name,
                         const bool is_directory = true );


    // --- Current Node Properties:
    [[nodiscard]]
    bool has_parent( void ) const;

    // --- Error Handling Methods:
    [[nodiscard]]
    bool has_errors( void ) const {
        return curr_error != Errors::NONE;
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

    // --- Error Handling:
    Errors curr_error = Errors::NONE;
};
