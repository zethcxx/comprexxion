#include <iostream>
#include "read_config/lexer.hpp"
#include "read_config/parser.hpp"

auto main() -> int
{
    /* ------------------------------------------------- */
    /*                      Example                      */
    /* ------------------------------------------------- */
    Lexer  lexer  { "comprexxion.txt" };
    Parser parser { lexer };

    if (parser.has_errors())
        return EXIT_FAILURE;

    std::cout << "~~~~~~~~ CONFIG ~~~~~~~~~\n";
    parser.print_config();

    return EXIT_SUCCESS;
}
