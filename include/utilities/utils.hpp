#pragma once

#include <cstddef>
#include <print>

namespace utils {
    void memdump (
        const void* const address,
        const std::size_t &limit
    ) noexcept;
}
