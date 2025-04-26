#include "loadcfg.hpp"
#include <cstddef>
#include <cstdlib>
#include <span>
#include <ranges>

auto main( int argc, char *argv[] ) -> int
{
    /* Parse command line arguments */
    auto args = std::span( argv, std::size_t(argc) )
            | std::views::transform( []( const char *arg ) {
                return std::string_view( arg );
            });

    if ( args.size() == 1 ) {
        cfg::loadcfg();

    } else if ( args.size() == 3 and args[1] == "-c" ) {
        cfg::loadcfg( args[2] );

    } else {
        cfg::usage();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
