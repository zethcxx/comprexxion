#pragma once

#include "lexer.hpp"
#include "token.hpp"
#include "tree.hpp"

#include <memory>
#include <string_view>
#include <map>
#include <utility>
#include <variant>
#include <print>


class Parser {
public:
    // --- Constructor:
    Parser ( Lexer &_lexer );


    // --- Main members:
    Lexer &lexer;
    Token  token;


    // --- Error state:
    bool has_errors = false;


    // --- Types:
    using TOKEN = Token::Type;
    // +
    using Identifier_value = std::variant <
            std::string,
            std::int64_t,
            std::shared_ptr<DirTree>
        >;
    // +
    using Identifiers_map  = std::map<
            std::string_view,
            std::pair<
                TOKEN,
                Identifier_value
            >
        >;


    // --- Main Identifiers:
    Identifiers_map identifiers_on_top {
    /* "<identifier>" { <type>, <default_value> } */
        {
            "project_name"  , {
                TOKEN::STRING,
                get_current_dir_name()
            }
        },
        {
            "project_root"  , {
                TOKEN::STRING,
                get_current_dir_path()
            }
        },
        {
            "compress_type" , {
                TOKEN::STRING,
                std::string ( "gzip" )
            }
        },
        {
            "compress_level", {
                TOKEN::VALID_NUMBER,
                std::int32_t( 4 )
            }
        },
        {
            "structure"       , {
                TOKEN::PATHS_BLOCK,
                /* By default, the entire current directory is included */
                std::make_shared<DirTree>(DirTree{})
            }
        },
    };


    // --- Helpers Methods:
    std::string get_current_dir_name( void );
    std::string get_current_dir_path( void );

    // --- Main Method:
    void parsing( void );

    // --- Helpers Methods:
    void skip_empty_lines( void );
    void print_config    ( void );
    void advance         ( void );
    // +
    bool is_token( const Token::Type &expected_token ) const;
    // +
    std::optional<std::int32_t> parse_int32( std::string_view str ) const;
    // +
    template<typename... Args>
    void report(
        std::format_string<Args...> format_str,
        Args&&... args
    ) {
        namespace fs = std::filesystem;
        this->has_errors = true;

        std::println( stderr, "File \"{}:{}:{}\"",
            fs::absolute( lexer.filepath ).string(),
            token.get_line    (),
            token.get_column  ()
        );

        const auto formatted = std::format(
            format_str,
            std::forward<Args>(args)...
        );

        std::println( stderr, "\x1b[1;31mError\x1b[0m: {}", formatted );
    }
};
