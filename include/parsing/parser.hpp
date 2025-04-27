#pragma once

// --- My Includes:
#include "parsing/lexer.hpp"
#include "parsing/token.hpp"
#include "parsing/tree.hpp"

// --- External Includes:
#include <fmt/core.h>

// --- Standard Includes:
#include <memory>
#include <string_view>
#include <map>
#include <utility>
#include <variant>
#include <format>


class Parser {
public:
    // --- Error Handling Methods:
    [[nodiscard]]
    bool has_errors( void ) const;


    // --- Debugging Methods:
    void print_config( void );


    // Types
    using Identifier_value = std::variant <
            std::string,
            std::int64_t,
            std::shared_ptr<DirTree>
        >;
    // +
    using Identifiers_map  = std::map<
            std::string_view,
            std::pair<
                Token::Type,
                Identifier_value
            >
        >;

            // --- Constructor:
    Parser ( Lexer &_lexer, Identifiers_map &_main_identifiers );


private:
    // --- Main members:
    Lexer &lexer;
    Token  token;

    // --- Error state:
    bool _has_errors = false;


    // --- Main Identifiers:
    Identifiers_map  &main_identifiers;


    // --- Helpers Methods:
    std::string get_lowercase( std::string_view str );


    // --- Utility Methods:
    void advance         ( void );
    void backward        ( void );
    void skip_empty_lines( void );
    bool parsing         ( void );


    // --- Token Checking:
    bool is_token( const Token::Type &expected_token ) const;


    // --- Parsing Methods:
    bool parse_paths_block( Identifier_value &raw_value );


    // --- Data Validation:
    std::optional <std::int32_t>
    parse_int32       ( std::string_view string     ) const;
    // +
    std::optional <Identifier_value>
    validate_data_type( std::string_view identifier );


    // Reporting Methods:
    template <typename... Args>
    bool report_error( std::format_string <Args...> format_str,
                       Args&&... args
    ) {
        namespace fs = std::filesystem;
        _has_errors = true;

        fmt::println( stderr, "File \"{}:{}:{}\"",
            fs::absolute( lexer.filepath ).string(),
            token.get_line    (),
            token.get_column  ()
        );

        const auto formatted = std::format(
            format_str,
            std::forward<Args>(args)...
        );

        fmt::println( stderr, "\x1b[1;31mError\x1b[0m: {}", formatted );
        return false;
    }
    // +
    template <typename... Args>
    bool report_error( const Token &temp_token,
                       std::format_string <Args...> format_str,
                       Args&&... args
    ) {
        const auto &last_token = this->token;

        /* swap token for reporting */
        token = temp_token;

        report_error<Args...>( format_str, std::forward<Args>(args)... );

        /* Restore the original token */
        token = last_token;
        return false;
    }
};
