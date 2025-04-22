#include "lexer.hpp"
#include <cstring>
#include <print>

Token Lexer::make_token(
    const Token::Type  type,
    const std::string &value
) const {
    return { type, line, column - value.length(), value };
}


bool Lexer::load_prev_buffer() {
    using std::streamoff, std::streampos;

    if ( not file or eof_flag )
        return false;

    streamoff back    = static_cast<streamoff>(BUFFER_SIZE * 2);
    streampos new_pos = file_pos - back;

    if (new_pos < 0)
        return false;

    file.seekg( new_pos );

    if ( not file )
        return false;

    return fill_buffer();
}


bool Lexer::fill_buffer() {
    if ( not file || eof_flag )
        return false;

    file_pos = file.tellg();

    file.read( buffer.data(), BUFFER_SIZE );

    buffer_len = static_cast<std::size_t>( file.gcount());
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

    curr_char = buffer.at( buffer_pos );

    buffer_pos++;
    column++;
}


void Lexer::backward() {
    if ( buffer_pos <= 0) return;

    --buffer_pos;
    --column    ;

    curr_char = buffer.at( buffer_pos );
}


Token Lexer::get_next_token() {
    using TOKEN = Token::Type;

    while ( not eof_flag ) {
        if ( column == 1 && is_indent_char( curr_char ))
            return parse_indent();

        if ( std::isspace( curr_char )) {
            if ( curr_char == '\n' ) {
                line++;
                advance();
                column = 1;

                return make_token( TOKEN::NEWLINE, "\\n" );
            }

            advance();
            continue;
        }


        if ( is_valid_char( curr_char ))
            return parse_identifier();

        else if ( is_digit( curr_char ))
            return parse_number();

        else if ( curr_char == '"' || curr_char == '\'' )
            return parse_string();

        else if ( curr_char == '#' )
            parse_comment();

        else
            return parse_symbol();
    }

    return make_token( TOKEN::END_OF_FILE, "EOF" );
}


Token Lexer::parse_number() {
    using enum Token::Type;
    std::string value;

    while ( is_digit( curr_char )) {
        value += curr_char;
        advance();
    }

    if ( is_valid_char( curr_char )) {
        while (
            is_valid_char( curr_char ) || is_digit( curr_char )
        ) {
            value += curr_char;
            advance();

            if ( eof_flag ) break;
        }

        return make_token( INVALID_NUMBER, value );
    }

    return make_token( VALID_NUMBER, value );
}


Token Lexer::parse_string() {
    using TOKEN = Token::Type;

    const char quote_type = curr_char;
    std::string value;
    advance();

    while ( not eof_flag && curr_char != quote_type ) {
        if ( curr_char == '\n' ) break;

        if ( curr_char == '\\' ) {
            value += curr_char;

            advance();
            if ( eof_flag )
                break;

            value += curr_char;
            continue;
        }

        value += curr_char;

        advance();
    }

    if ( eof_flag || curr_char != quote_type )
        return make_token( TOKEN::UNCLOSED_STRING, "\"" + value );

    advance();
    return make_token( TOKEN::STRING, value );
}


Token Lexer::parse_symbol() {
    using enum Token::Type;
    std::string symbol{ 1, curr_char };

    switch ( curr_char ) {
        case ':':
            advance();
            return make_token( ASSIGN, symbol );

        case '+':
            advance();
            return make_token( PATH_INDICATOR, symbol );

        default:
            advance();
            return make_token( SYMBOL, symbol );
    }
}


Token Lexer::parse_indent() {
    using enum Token::Type;

    std::string indent_value;

    bool has_spaces = false;
    bool has_tabs   = false;

    while ( is_indent_char( curr_char )) {
        if ( curr_char == ' '  ) has_spaces = true;
        if ( curr_char == '\t' ) has_tabs   = true;

        indent_value += curr_char;
        advance();
    }

    if ( has_spaces && has_tabs )
        return make_token( INDENT_MIXED, indent_value );

    else if ( has_spaces )
        return make_token( INDENT_SPACE , indent_value );

    else
        return make_token( INDENT_TAB   , indent_value );
}


void Lexer::parse_comment() {
    while ( curr_char != '\n' && not eof_flag )
        advance();
}


Token Lexer::parse_identifier() {
    std::string identifier;

    while (is_valid_char(curr_char)) {
        identifier += curr_char;
        advance();

        if ( eof_flag ) break;
    }

    return make_token( Token::Type::IDENTIFIER, identifier );
}


bool Lexer::is_valid_char ( const char &c ) {
    return std::isalpha( static_cast<unsigned char>( c )) || c == '_';
}


bool Lexer::is_digit ( const char &c ) {
    return std::isdigit( static_cast<unsigned char>( c ));
}


bool Lexer::is_indent_char ( const char &c ) {
    return (c == ' ' || c == '\t' );
}


Lexer::Lexer ( const std::filesystem::path &_filepath )
  : filepath { _filepath },
    file     { _filepath, std::ios::in | std::ios::binary }
{
    if ( file.is_open() ) {
        buffer.resize( BUFFER_SIZE, 0 );
        advance();
        return;
    }

    _has_errors = true;

    std::println( stderr, "File \"{}\"",
            std::filesystem::absolute( _filepath ).string()
        );

    std::println( stderr, "Error: {}", std::strerror( errno ));
}
