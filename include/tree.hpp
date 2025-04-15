#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class DirTree {
    private:

        struct Node {
            std::string name;
            bool is_directory;
            Node *parent;

            std::unordered_map< std::string, std::unique_ptr<Node> > childrens;

            Node (
                const std::string &_name, bool _is_directory, Node *_parent = nullptr
            );

        };

        std::unique_ptr<Node> root;
        Node* curr_node;

    public:
        DirTree();
        
        void go_to_parent( void );
        void print_tree  ( void );

        [[nodiscard]]
        bool has_parent( void ) const {
            return curr_node->parent != nullptr;
        }

        [[nodiscard]]
        bool go_to_child ( const std::string& name );

        [[nodiscard]]
        bool add_child( const std::string& name, bool is_directory = true );
};
