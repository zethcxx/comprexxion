// --- My Includes:
#include "loadcfg.hpp"
#include "parsing/lexer.hpp"
#include "parsing/parser.hpp"
#include "parsing/token.hpp"
#include "parsing/tree.hpp"


// --- External Includes:
#include <fmt/core.h>

// --- Standard Includes:
#include <span>
#include <ranges>
#include <string>
#include <string_view>
#include <filesystem>
#include <vector>

/* Internal Linkage */
namespace {

    // --- Usage Method:
    void usage( void ) {
        #ifdef _WIN32
            std::string executable_name = "comprexxion.exe";
        #else
            std::string executable_name = "comprexxion";
        #endif

        fmt::println( "Usage: {} -c <config file>", executable_name );
    }


    // --- Helper Methods:
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


    using TOKEN = Token::Type;

    // --- Main Identifiers:
    Parser::Identifiers_map identifiers_on_top {
    /* "<identifier>" { <type>, <default_value> } */
        {
            "project_name"  , {
                TOKEN::STRING,
                get_current_dir_name()
            }
        },
        {
            "project_root"  , {
                TOKEN::STRING,
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
                std::make_shared<DirTree>(DirTree{})
            }
        },
    };


    void create_structure( void ) {
        struct DirFrame {
            DirTree::children_node_t::const_iterator begin;
            DirTree::children_node_t::const_iterator end  ;
        };

        std::vector<DirFrame> stack;

        auto root_node = std::get<std::shared_ptr<DirTree>>(
            identifiers_on_top["structure"].second
        )->get_root();

        stack.push_back( DirFrame {
            .begin = root_node->children.begin(),
            .end   = root_node->children.end  ()
        });

        // TODO: implement recursive directory copy
    }
}


bool cfg::loadcfg( int argc, char *argv[] ) {

    /* Parse command line arguments */
    const auto args = std::span( argv, std::size_t(argc) )
            | std::views::transform( []( const char *arg ) {
                return std::string_view( arg );
            });

    std::string filepath { "comprexxion.txt" };

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
