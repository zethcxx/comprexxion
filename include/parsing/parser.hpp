#pragma once

// ---- LOCAL INCLUDES ----
//
#include "parsing/lexer.hpp"
#include "parsing/token.hpp"
#include "parsing/tree.hpp"


// ---- EXTERNAL INCLUDES ----
//
#include <fmt/core.h>


// ---- STANDARD INCLUDES ----
//
#include <memory>
#include <string_view>
#include <map>
#include <utility>
#include <variant>
#include <format>


class Parser {
public:
    // ---- ERROR HANDLING METHODS ----
    //
    [[nodiscard]]
    bool has_errors( void ) const;


    // ---- DEBUGGING METHODS -----
    //
    void print_config( void );


    // ---- MAIN TYPES ----
    //
    using ident_value_t = std::variant <
            std::string,
            std::int64_t,
            std::shared_ptr<DirTree>
        >;
    // +
    using ident_map_t  = std::map<
            std::string_view,
            std::pair<
                Token::Type,
                ident_value_t
            >
        >;


    // ---- CONSTRUCTOR ----
    //
    Parser ( Lexer &_lexer, ident_map_t &_main_identifiers );


private:
    // ---- MAIN MEMBERS ----
    //
    Lexer &lexer;
    Token  token;


    // ---- ERROR STATE ----
    //
    bool _has_errors = false;


    // ---- MAIN IDENTIFIERS ----
    //
    ident_map_t  &main_identifiers;


    // --- HELPERS METHODS ----
    //
    std::string get_lowercase( std::string_view str );


    // ---- UTILITY METHODS ----
    //
    void backward        ( void );
    void skip_empty_lines( void );
    bool advance         ( void );
    bool parsing         ( void );


    // ---- TOKEN CHECKING ----
    //
    bool is_token( const Token::Type &expected_token ) const;


    // ---- PARSING METHODS ----
    //
    bool parse_paths_block(ident_value_t &raw_value);
    // +
    std::optional <std::int32_t>
    parse_int32 ( std::string_view string ) const;


    // ---- VALIDATION OF DATA ----
    //
    std::optional <ident_value_t>
    validate_data_type( std::string_view identifier );


    // ---- REPORTING METHODS ----
    //
    template <typename... Args>
    bool report_error( std::format_string <Args...> format_str,
                       Args&&... args
    ) {
        namespace fs = std::filesystem;
        _has_errors = true;

        fmt::println( stderr, "File \"{}:{}:{}\"",
            fs::absolute( lexer.filepath ).string(),
            token.get_line  (),
            token.get_column()
        );

        const auto formatted = std::format(
            format_str,
            std::forward<Args>(args)...
        );

        fmt::println( stderr, "\x1b[1;31mError\x1b[0m: {}", formatted );
        return false;
    }
    // + [ OVERLOAD FOR CUSTOM TOKEN ]
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
