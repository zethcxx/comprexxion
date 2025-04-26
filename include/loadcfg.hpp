#pragma once

// --- Standard Includes:
#include <string_view>

namespace cfg {
    bool loadcfg( std::string_view filepath = "comprexxion.txt" );
    void usage  ( void );
}
