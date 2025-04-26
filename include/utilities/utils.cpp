// --- My Includes:
#include "utilities/utils.hpp"

// --- External Includes:
#include <fmt/core.h>

// --- Standard Includes:
#include <cstdint>
#include <string>


__attribute__((no_sanitize("address")))
void utils::memdump( const void *const address,
                     const std::size_t &limit
) noexcept {
    const auto *mem = static_cast<const std::uint8_t*>( address );

    std::uintptr_t
        begin_ptr = reinterpret_cast<std::uintptr_t>( address ),
        curr_ptr  = begin_ptr,
        end_ptr   = begin_ptr + limit;

    std::size_t offset {};

    std::string string (16, ' ');

    for (; curr_ptr < end_ptr; curr_ptr += 16) {
        std::uint8_t str_idx {0};

        fmt::print("\x1b[38;5;12m{:#x}\x1b[0;0m: ", curr_ptr);

        for (; offset < limit ; offset++) {
            const std::uint8_t byte = mem[offset];

            fmt::print("{:02x} ", byte);

            string.at( str_idx ) =
                ( byte >= 33 && byte <= 126 )
                    ? char(byte) /* if is ascii    */
                    : '.' ;      /* if isn't ascii */

            str_idx++;

            /* If next offset is a multiple of 16 */
            if ((( offset + 1 ) & 15) == 0) {
                offset++;
                break;
            }
        }

        std::fill( string.begin() + str_idx, string.end(), ' ' );

        /* for left padding */
        if ( offset == limit )
            fmt::print( "{:>{}}",
                "",
                ((curr_ptr + 16) - end_ptr) * 3
            );

        fmt::println(" {}", string);
    }
}
