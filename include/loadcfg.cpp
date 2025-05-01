// ---- LOCAL INCLUDES ----
//
#include "loadcfg.hpp"
#include "parsing/lexer.hpp"
#include "parsing/parser.hpp"
#include "parsing/token.hpp"
#include "parsing/tree.hpp"


// ---- EXTERNAL INCLUDES ----
//
#include <fmt/core.h>


// ---- STANDARD INCLUDES ----
//
#include <span>
#include <ranges>
#include <string>
#include <string_view>
#include <filesystem>
#include <vector>


// ---- INTERNAL LINKAGES ----
//
namespace {

    void usage( void ) {
        #ifdef _WIN32
            constexpr std::string_view executable_name = "comprexxion.exe";
        #else
            constexpr std::string_view executable_name = "comprexxion";
        #endif

        fmt::println( "Usage: {} -c <config file>", executable_name );
    }


    // ---- HELPER FUNCTIONS ----
    //
    std::string get_current_dir_name( void ) {
        namespace fs = std::filesystem;

        const auto path_name = fs::absolute(".").parent_path().filename().string();

        /* if is the is the root directory */
        if ( path_name.empty() )
            return "root_fs";
        else
            return path_name;
    }
    // +
    std::string get_current_dir_path( void ) {
        namespace fs = std::filesystem;

        return fs::absolute(".").parent_path().string();
    }


    // ---- TYPEDEFS ----
    //
    using TOKEN = Token::Type;


    // ---- MAIN IDENTIFIERS ----
    //
    Parser::ident_map_t identifiers_on_top {
    /* "<identifier>" { <type>, <default_value> } */
        {
            "project_name"  , {
                TOKEN::BASENAME,
                get_current_dir_name()
            }
        },
        {
            "project_root"  , {
                TOKEN::PATH,
                get_current_dir_path()
            }
        },
        {
            "compress_type" , {
                TOKEN::STRING,
                std::string ( "gzip" )
            }
        },
        {
            "compress_level", {
                TOKEN::VALID_NUMBER,
                std::int32_t( 4 )
            }
        },
        {
            "structure"       , {
                TOKEN::PATHS_BLOCK,
                /* By default, the entire current directory is included */
                std::make_shared<DirTree>()
            }
        },
    };


    std::filesystem::path get_full_path_of( const DirTree& tree ) {
        const auto &node = tree.get_curr_node();
        auto *current_node = &node;

        std::vector<std::string_view> path_parts;

            while ( current_node->has_parent() ) {
            path_parts.push_back( current_node->get_name() );
            current_node = current_node->get_parent();
        }

        std::filesystem::path full_path;

        for ( auto it = path_parts.rbegin(); it != path_parts.rend(); it++ ) {
            full_path /= *it;
        }

        return full_path;
    }


    void create_structure( void ) {
        namespace fs = std::filesystem;

        struct DirFrame {
            DirTree::children_node_t::const_iterator begin;
            DirTree::children_node_t::const_iterator end  ;
        };

        std::vector<DirFrame> stack;

        const auto tree_ptr = std::get<std::shared_ptr<DirTree>>(
                identifiers_on_top["structure"].second
            );

        const auto &tree = *tree_ptr;

        const auto &root_node = tree.get_root();

        const auto &project_name = std::get<std::string>(
            identifiers_on_top["project_name"].second
        );

        fs::create_directory( project_name );

        stack.push_back( DirFrame {
            .begin = root_node.children.begin(),
            .end   = root_node.children.end  ()
        });


        while ( not stack.empty() ) {
            auto &[it, it_end] = stack.back();

            if ( it == it_end ) {
                stack.pop_back();
                continue;
            }

            const auto &node = *( it->second );

            const fs::path source_path = get_full_path_of(tree);
            const fs::path target_path = fs::path( project_name ) / source_path;

            if ( not fs::exists( source_path ) ) {
                fmt::println("File not found: {}", source_path.string());
                ++it;
                continue;
            }


            it++;

            if ( node.is_directory() ) {
                fs::create_directory( target_path );
                fmt::println("created: {}", target_path.string());

                stack.push_back( DirFrame {
                    .begin = node.children.begin(),
                    .end   = node.children.end  ()
                });

            } else {
                fs::copy_file( source_path, target_path );
                fmt::println("copied: {}", target_path.string());
            }
        }
    }
}


bool cfg::loadcfg( int argc, char *argv[] ) {

    /* Parse command line arguments */
    const auto args = std::span( argv, std::size_t(argc) )
            | std::views::transform( []( const char *arg ) {
                return std::string_view( arg );
            });

    std::string filepath { "comprexxion.txt" };

    // TODO: Add support for the -f (force) flag to remove the previous project if it exists

    /* Check if the filepath is specified */
    if ( args.size() == 3 and args[1] == "-c" ) {
        filepath = args[2];

    } else if ( args.size() != 1 ) {
        usage();
        return false;
    }


    Lexer  lexer  { filepath };
    Parser parser { lexer, identifiers_on_top };


    if ( lexer.has_errors() || parser.has_errors() ) {
        return false;
    }


    #ifdef DEBUG
        parser.print_config();
    #endif


    create_structure();


    return true;
}
