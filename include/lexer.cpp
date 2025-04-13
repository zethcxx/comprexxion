#include "lexer.hpp"
#include <cstring>
#include <print>

bool Lexer::fill_buffer() {
    if ( not file ) return false;

    file.read( buffer.data(), BUFFER_SIZE );

    buffer_len = static_cast<std::size_t>(file.gcount());
    buffer_pos = 0;

    return buffer_len > 0;
}


void Lexer::advance() {
    if ( buffer_pos >= buffer_len ) {
        if ( not fill_buffer()) {
            eof_flag = true;
            return;
        }
    }

    curr_char = buffer.at(buffer_pos);

    buffer_pos++;
    column++;
}


void Lexer::backward() {
    if ( buffer_pos > 0 ) {
        --buffer_pos;
        --column;

        curr_char = buffer.at(buffer_pos);
    }
}


Token Lexer::get_next_token() {
    using TOKEN = Token::Type;

    while (not eof_flag) {
        if (column == 1 && is_indent_char(curr_char)) {
            return parse_indent();
        }

        if (std::isspace(curr_char)) {
            if (curr_char == '\n') {
                line++;
                advance();
                column = 1;

                return make_token(TOKEN::NEWLINE, "\\n");
            }

            advance();
            continue;
        }


        if (is_identifier_char(curr_char)) {
            return parse_identifier();
        }

        if (is_number(curr_char)) {
            return parse_number();
        }

        if (curr_char == '"' || curr_char == '\'') {
            return parse_string();
        }

        if (curr_char == '#') {
            parse_comment();
            continue;
        }

        return parse_symbol();
    }

    return make_token(TOKEN::END_OF_FILE, "");
}


Token Lexer::parse_number() {
    std::string number;

    while (is_number(curr_char)) {
        number += curr_char;
        advance();
    }

    return make_token(Token::Type::NUMERIC, number);
}


Token Lexer::parse_string() {
    char quote_type = curr_char;
    std::string value, bad_value;
    bad_value += quote_type;
    advance();

    while (not eof_flag) {
        if (curr_char == '\n') break;

        if (curr_char == '\\') {
            advance();
            if (eof_flag) break;
            value += '\\';
            value += curr_char;
        } else if (curr_char == quote_type) {
            break;
        } else {
            value += curr_char;
        }

        advance();
    }

    bad_value += value;

    if (eof_flag || curr_char != quote_type) {
        return make_token(Token::Type::UNCLOSED_STRING, bad_value);
    }

    advance();
    return make_token(Token::Type::STRING, value);
}


Token Lexer::parse_symbol() {
    using TOKEN = Token::Type;
    std::string symbol(1, curr_char);

    switch (curr_char) {
        case ':':
            advance();
            return make_token(TOKEN::ASSIGN, symbol);

        case '+':
            advance();
            return make_token(TOKEN::PATH_INDICATOR, symbol);

        default:
            advance();
            return make_token(TOKEN::SYMBOL, symbol);
    }
}


Token Lexer::parse_indent() {
    std::string indent_string;
    bool exist_space = false, exist_tab = false;

    while (is_indent_char(curr_char)) {
        if (curr_char == ' ')  exist_space = true;
        if (curr_char == '\t') exist_tab   = true;

        indent_string += curr_char;
        advance();
    }

    if (exist_space && exist_tab)
        return make_token(Token::Type::INDENT_MIXTED, indent_string);

    if (exist_tab)
        return make_token(Token::Type::INDENT_TAB, indent_string);

    return make_token(Token::Type::INDENT_SPACE, indent_string);
}


void Lexer::parse_comment() {
    while (curr_char != '\n' && not eof_flag)
        advance();
}


Token Lexer::parse_identifier() {
    std::string identifier;

    while (is_identifier_char(curr_char)) {
        identifier += curr_char;
        advance();

        if ( eof_flag ) break;
    }

    return make_token(Token::Type::IDENTIFIER, identifier);
}


bool Lexer::is_identifier_char ( const char &c ) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z' ) || (c == '_');
}


bool Lexer::is_number ( const char &c ) {
    return (c >= '0' && c <= '9' );
}


bool Lexer::is_indent_char ( const char &c ) {
    return (c == ' ' || c == '\t' );
}


Lexer::Lexer (
    const std::filesystem::path &_filepath
)
  : file   { _filepath, std::ios::in | std::ios::binary }
{
    if ( not file.is_open()) {
        std::println(stderr, "File \"{}\"", std::filesystem::absolute(_filepath).string());
        std::println(stderr, "Error: {}", std::strerror( errno ));
        has_errors = true;
    }

    buffer.resize(BUFFER_SIZE, 0);
    advance();
}
