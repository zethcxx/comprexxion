#include "token.hpp"

std::string_view Token::get_typestr( std::optional<Type> token_type ) const {
    using TOKEN = Token::Type;

    switch ( token_type.value_or( this->type )) {
        case TOKEN::BEGIN_OF_FILE  : return "BEGIN_OF_FILE"  ;
        case TOKEN::IDENTIFIER     : return "IDENTIFIER"     ;
        case TOKEN::STRING         : return "STRING"         ;
        case TOKEN::UNCLOSED_STRING: return "UNCLOSED_STRING";
        case TOKEN::VALID_NUMBER   : return "VALID_NUMBER"   ;
        case TOKEN::INVALID_NUMBER : return "INVALID_NUMBER" ;
        case TOKEN::NEWLINE        : return "NEWLINE"        ;
        case TOKEN::ASSIGN         : return "ASSIGN"         ;
        case TOKEN::SYMBOL         : return "SYMBOL"         ;
        case TOKEN::PATH_INDICATOR : return "PATH_INDICATOR" ;
        case TOKEN::PATHS_BLOCK    : return "PATHS_BLOCK"    ;
        case TOKEN::INDENT_TAB     : return "INDENT_TAB"     ;
        case TOKEN::INDENT_SPACE   : return "INDENT_SPACE"   ;
        case TOKEN::INDENT_MIXED   : return "INDENT_MIXED"   ;
        case TOKEN::END_OF_FILE    : return "END_OF_FILE"    ;
        default:
            return "Unknow";
    }
}


Token::Token (
    const Token::Type &_type  ,
    const std::size_t &_line  ,
    const std::size_t &_column,
    const std::string &_value
)
  : type   { _type   },
    line   { _line   },
    column { _column },
    value  { _value  }
{}


Token::Token ()
  : type   { Type::BEGIN_OF_FILE },
    line   { 1 },
    column { 1 },
    value  { "BEGIN_OF_FILE" }
{}
