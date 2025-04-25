#pragma once

// --- My Includes:
#include "parsing/token.hpp"

// --- Standard Includes:
#include <fstream>
#include <filesystem>
#include <vector>

class Lexer {
public:
    // --- Constructors:
    explicit Lexer(const std::filesystem::path &_filepath);

    // --- Input File Path:
    std::filesystem::path filepath;


    // --- Main Methods:
    Token get_next_token();
    Token get_curr_token();


    // --- Error Handling:
    [[nodiscard]]
    bool has_errors( void ) const {
        return _has_errors;
    }


    // --- Helper Methods:
    static bool is_valid_char ( const char &c );
    static bool is_digit      ( const char &c );
    static bool is_indent_char( const char &c );


private:

    // --- File Stream:
    std::ifstream file;

    // --- Error Flag:
    bool _has_errors = false;

    // -- Lexer State:
    std::size_t line   = 1;
    std::size_t column = 0;
    // +
    Token curr_token;
    char  curr_char ;
    bool  eof_flag  ;


    // --- Buffer State:
    static constexpr std::size_t BUFFER_SIZE = 4096;
    std::vector<char> buffer;
    // +
    std::size_t    buffer_pos = 0;
    std::size_t    buffer_len = 0;
    std::streampos file_pos   = 0;


    // --- Buffer Handling:
    bool fill_buffer     ( void );
    bool load_prev_buffer( void );


    // --- Navigation:
    void advance ( void );
    void backward( void );


    // --- Token Parsers:
    Token parse_identifier( void );
    Token parse_indent    ( void );
    Token parse_number    ( void );
    Token parse_string    ( void );
    Token parse_symbol    ( void );
    // +
    void parse_comment( void );


    // --- Creation Token:
    Token make_token(
        const Token::Type  type,
        const std::string &value
    );
};
