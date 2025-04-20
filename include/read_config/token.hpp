#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

class Token {
public:
    // --- Tokens Types
    enum class Type : std::uint8_t {
        BEGIN_FILE     ,
        IDENTIFIER     ,
        STRING         ,
        UNCLOSED_STRING,
        VALID_NUMBER   ,
        INVALID_NUMBER ,
        NEWLINE        ,
        ASSIGN         ,
        SYMBOL         ,
        PATH_INDICATOR ,
        PATHS_BLOCK    ,
        INDENT_TAB     ,
        INDENT_SPACE   ,
        INDENT_MIXED   ,
        END_OF_FILE
    };


    // --- Get Methods
    [[nodiscard]]
    const std::string &get_value() const { return this->value; }
    // +
    [[nodiscard]]
    std::size_t get_line   () const { return this->line  ; }
    // +
    [[nodiscard]]
    std::size_t get_column () const { return this->column; }
    // +
    [[nodiscard]]
    Token::Type get_type   () const { return this->type  ; }
    // +
    [[nodiscard]]
    std::string_view get_typestr (
        std::optional<Type> token_type = std::nullopt
    ) const;


    // --- Constructors
    Token (
        const Token::Type &_type  ,
        const std::size_t &_line  ,
        const std::size_t &_column,
        const std::string &_value
    );
    // +
    Token();

private:
    // --- Token Attributes
    Token::Type type  ;
    std::size_t line  ;
    std::size_t column;
    std::string value ;
};
