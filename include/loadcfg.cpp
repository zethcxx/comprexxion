// --- My Includes:
#include "loadcfg.hpp"
#include "parsing/lexer.hpp"
#include "parsing/parser.hpp"

// --- External Includes:
#include <fmt/core.h>

// --- Standard Includes:
#include <span>
#include <ranges>
#include <string>
#include <string_view>

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
        cfg::usage();
        return false;
    }


    Lexer  lexer  { filepath };
    Parser parser { lexer };

    if ( lexer.has_errors() || parser.has_errors() ) {
        return false;
    }


    #ifdef DEBUG
        parser.print_config();
    #endif


    return true;
}


void cfg::usage( void ) {
    #ifdef _WIN32
        std::string executable_name = "comprexxion.exe";
    #else
        std::string executable_name = "comprexxion";
    #endif

    fmt::println( "Usage: {} -c <config file>", executable_name );
}
