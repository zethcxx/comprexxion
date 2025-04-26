// --- My Includes:
#include "loadcfg.hpp"
#include "parsing/lexer.hpp"
#include "parsing/parser.hpp"

// --- External Includes:
#include <fmt/core.h>

// --- Standard Includes:
#include <string_view>

bool cfg::loadcfg( std::string_view filepath ) {
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
