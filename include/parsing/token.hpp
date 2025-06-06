#pragma once

// ---- STANDARD INCLUDES ----
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>


class Token {
public:

    // ---- TYPES ----
    //
    enum class Type : std::uint8_t {
        BEGIN_OF_FILE  ,
        IDENTIFIER     ,
        STRING         ,
        BASENAME       ,
        PATH  ,
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


    // ---- GETTERS ----
    //
    [[nodiscard]] const std::string& get_value () const;
    [[nodiscard]] std::size_t        get_line  () const;
    [[nodiscard]] std::size_t        get_column() const;
    [[nodiscard]] Type               get_type  () const;
    // +
    std::string_view get_typestr (
        std::optional<Type> token_type = std::nullopt
    ) const;


    // ---- CONSTRUCTORS ----
    //
    Token ( const Token::Type &_type  ,
            const std::size_t &_line  ,
            const std::size_t &_column,
            const std::string &_value  );
    // +
    Token();


private:

    Token::Type type  ;
    std::size_t line  ;
    std::size_t column;
    std::string value ;
};
