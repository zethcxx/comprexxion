#pragma once

// ---- LOCAL INCLUDES ----
//
#include "parsing/token.hpp"


// ---- STANDARD INCLUDES ----
//
#include <fstream>
#include <filesystem>
#include <vector>


class Lexer {
public:
    // ---- CONSTRUCTORS ----
    //
    explicit Lexer(const std::filesystem::path &_filepath);


    // ---- INPUT FILE PATH ----
    //
    std::filesystem::path filepath;


    // ---- MAIN METHODS ----
    //
    Token get_next_token();
    Token get_curr_token();


    // ---- ERROR HANDLING ----
    //
    [[nodiscard]]
    bool has_errors( void ) const;


    // ---- HELPER METHODS ----
    //
    static bool is_identifier_char ( const char &c );
    static bool is_digit           ( const char &c );
    static bool is_indent_char     ( const char &c );


private:
    // ---- FILE STREAM ----
    //
    std::ifstream file;


    // ---- ERROR STATE ----
    //
    bool _has_errors = false;


    // ---- LEXER STATE ----
    //
    std::size_t line   = 1;
    std::size_t column = 0;
    // +
    Token curr_token {};
    // +
    char  curr_char { '\0'  };
    bool  eof_flag  { false };


    // ---- BUFFER STATE ----
    //
    static constexpr std::size_t BUFFER_SIZE = 4096;
    std::vector<char> buffer;
    // +
    std::size_t    buffer_pos = 0;
    std::size_t    buffer_len = 0;
    std::streampos file_pos   = 0;


    // ---- BUFFER HANDLING ----
    //
    bool fill_buffer     ( void );
    bool load_prev_buffer( void );


    // ---- NAVIGATION METHODS ----
    //
    void advance ( void );


    // ---- SCANNING METHODS ----
    //
    Token tokenize_identifier( void );
    Token tokenize_indent    ( void );
    Token tokenize_number    ( void );
    Token tokenize_string    ( void );
    Token tokenize_symbol    ( void );
    // +
    void skip_comment( void );


    // --- Creation Token:
    Token make_token(
        const Token::Type  type,
        const std::string &value
    );
};
