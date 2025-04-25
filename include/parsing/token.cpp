// --- My Includes
#include "token.hpp"


std::string_view Token::get_typestr( std::optional<Type> token_type ) const {
    using enum Type;

    switch ( token_type.value_or( this->type )) {
        case BEGIN_OF_FILE  : return "BEGIN_OF_FILE"  ;
        case IDENTIFIER     : return "IDENTIFIER"     ;
        case STRING         : return "STRING"         ;
        case UNCLOSED_STRING: return "UNCLOSED_STRING";
        case VALID_NUMBER   : return "VALID_NUMBER"   ;
        case INVALID_NUMBER : return "INVALID_NUMBER" ;
        case NEWLINE        : return "NEWLINE"        ;
        case ASSIGN         : return "ASSIGN"         ;
        case SYMBOL         : return "SYMBOL"         ;
        case PATH_INDICATOR : return "PATH_INDICATOR" ;
        case PATHS_BLOCK    : return "PATHS_BLOCK"    ;
        case INDENT_TAB     : return "INDENT_TAB"     ;
        case INDENT_SPACE   : return "INDENT_SPACE"   ;
        case INDENT_MIXED   : return "INDENT_MIXED"   ;
        case END_OF_FILE    : return "END_OF_FILE"    ;
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
