#pragma once

#include "token.hpp"
#include <fstream>
#include <filesystem>
#include <vector>

class Lexer {
    private:
        std::ifstream file;

        std::size_t line       = 1;
        std::size_t column     = 0;
        std::size_t buffer_pos = 0;
        std::size_t buffer_len = 0;

        Token curr_token;

        char curr_char;
        bool eof_flag ;

        static constexpr std::size_t BUFFER_SIZE = 4096;

        std::vector<char> buffer;

        bool fill_buffer  ( void );
        void parse_comment( void );
        void advance      ( void );
        void backward     ( void );

        Token parse_identifier( void );
        Token parse_indent    ( void );
        Token parse_number    ( void );
        Token parse_string    ( void );
        Token parse_symbol    ( void );

        Token make_token( Token::Type type, const std::string &value ) {
            return { type, line, column - value.length(), value };
        }

    public:
        bool has_errors = false;

        explicit Lexer (
            const std::filesystem::path &_filepath
        );

        static bool is_identifier_char (const char &c);
        static bool is_number          (const char &c);
        static bool is_indent_char     (const char &c);

        Token get_next_token( void );
        Token get_curr_token( void );
};
