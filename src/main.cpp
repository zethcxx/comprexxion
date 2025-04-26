#include "loadcfg.hpp"
#include <cstdlib>

int main( int argc, char *argv[] )
{
    if ( not cfg::loadcfg(argc, argv))
        return EXIT_FAILURE;


    return EXIT_SUCCESS;
}
