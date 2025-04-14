#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

class Token {
    public:
        enum class Type : std::uint8_t {
            BEGIN_FILE     ,
            IDENTIFIER     ,
            STRING         ,
            UNCLOSED_STRING,
            NUMBER         ,
            INVALID_NUMBER ,
            NEWLINE        ,
            ASSIGN         ,
            SYMBOL         ,
            PATH_INDICATOR ,
            PATHS_BLOCK    ,
            INDENT_TAB     ,
            INDENT_SPACE   ,
            INDENT_MIXTED  ,
            END_OF_FILE
        };


        /* ------------ GET METHODS ------------ */
        [[nodiscard]]
        const std::string &get_value() const { return this->value; }


        [[nodiscard]]
        std::size_t get_line   () const { return this->line  ; }


        [[nodiscard]]
        std::size_t get_columm () const { return this->column; }


        [[nodiscard]]
        Token::Type get_type   () const { return this->type  ; }


        [[nodiscard]]
        std::string_view get_typestr (
            std::optional<Type> token_type = std::nullopt
        ) const;


        /* ---------- CONSTRUCTORS ---------- */
        Token (
            const Token::Type &_type  ,
            const std::size_t &_line  ,
            const std::size_t &_column,
            const std::string &_value
        );

        Token();

    private:
        /* -- MEMBERS -- */
        Token::Type type  ;
        std::size_t line  ;
        std::size_t column;
        std::string value ;
};
