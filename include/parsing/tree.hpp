#pragma once

// ---- STANDARD INCLUDES ----
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>


class DirTree {
public:

    // ---- CONSTRUCTORS ----
    //
    DirTree();
    DirTree( const std::string& _root_name );


    // ---- ERRORS TYPES ----
    //
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


    // ---- NODE TYPES ----
    //
    enum class NodeType : std::uint8_t {
        IS_DIRECTORY,
        IS_FILE
    };


    // ---- DEBUG METHOD  ----
    //
    void print_tree ( size_t initial_indent = 0 ) const noexcept;


    // ---- METHODS FOR CURRENT NODE ----
    //
    [[nodiscard]]
    Errors go_to_parent( void );
    Errors go_to_child ( const std::string& name );
    Errors add_child   ( const std::string& name,
                         NodeType type = NodeType::IS_DIRECTORY );
    // +
    [[nodiscard]]
    bool has_parent( void ) const;
    // +
    void ascend_levels( std::size_t levels );


    // ---- ERROR HANDLING ----
    //
    [[nodiscard]]
    bool has_errors( void ) const {
        return curr_error != Errors::NONE;
    }
    // +
    [[nodiscard]]
    Errors get_error( void ) const {
        return curr_error;
    }


private:

    struct Node {

        // ---- BASIC PROPERTIES ----
        //
        std::string name;
        NodeType    type;
        Node     *parent;

        // ---- CHILDREN MANAGEMENT ----
        //
        std::unordered_map<
            std::string /* name of child */,
            std::unique_ptr<Node>> children {};

        // ---- HELPER METHODS ----
        //
        [[nodiscard]]
        bool is_directory( void ) const;

        // ---- CONSTRUCTOR ----
        //
        Node ( const std::string &_name,
               NodeType _type,
               Node *_parent = nullptr );

        // ---- PROHIBITED COPY ----
        //
        Node( const Node& ) = delete;
        Node& operator=( const Node& ) = delete;
    };


    // ---- ROOT OF TREE ----
    //
    std::unique_ptr<Node> root;

    // ---- CURRENT NODE ----
    //
    Node* curr_node;

    // ---- ERROR STATE ----
    //
    Errors curr_error = Errors::NONE;

    // ---- COMPLETE CURRENT NODE PATH ----
    //
    std::filesystem::path full_path;

public:

    // ---- TYPES ----
    using children_node_t = decltype ( Node::children );

    // ---- GET METHODS ----
    const Node& get_root     ( void ) const;
    const Node& get_curr_node( void ) const;

    // --- Actions:
    Errors select_all_of( const Node& node );
};
