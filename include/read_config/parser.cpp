#include "parser.hpp"
#include "token.hpp"

#include <algorithm>
#include <charconv>
#include <filesystem>
#include <cstdint>
#include <string>
#include <unordered_set>

bool Parser::is_token( const Token::Type &expected_token ) const {
    return ( token.get_type() == expected_token );
}


void Parser::advance() {
    this->token = lexer.get_next_token();
}


std::string Parser::get_current_dir_name( void ) {
    namespace fs = std::filesystem;

    return fs::absolute(".").parent_path().filename().string();
}


std::string Parser::get_current_dir_path( void ) {
    namespace fs = std::filesystem;

    return fs::absolute(".").parent_path().string();
}


std::optional<std::int32_t> Parser::parse_int32( std::string_view str ) const {
    std::int32_t result;
    const auto [ptr, ec] = std::from_chars( str.begin(), str.end(), result );

    if ( ec == std::errc() && ptr == str.end() )
        return result;

    return std::nullopt;
}


void Parser::skip_empty_lines() {
    while ( is_token( TOKEN::NEWLINE )) advance();
}


void Parser::parsing() {
    using enum Token::Type;

    std::unordered_set<std::string> identifiers_used;

    auto is_duplicate = [&]( const std::string &identifier ) -> bool {
        if ( identifiers_used.contains( identifier ) ) return true;

        identifiers_used.insert( identifier );
        return false;
    };

    if ( is_token( BEGIN_FILE )) advance();

    while ( not is_token( END_OF_FILE )) {

        skip_empty_lines();

        /* only empty lines */
        if ( is_token( END_OF_FILE )) break;


        if ( not is_token( IDENTIFIER )) {
            report(
                "Identifier expected, but got '{}'",
                token.get_value()
            );
            break;
        }


        const std::string identifier = token.get_value();


        if ( not identifiers_on_top.contains( identifier) ) {
            report( "Identifier Unknow '{}'", identifier);
            break;
        }


        const TOKEN allowed_type = identifiers_on_top.at( identifier ).first;


        if ( is_duplicate( identifier )) {
            report("Duplicate identifier '{}'", identifier);
            break;

        } else advance();


        if ( not is_token( ASSIGN )) {
            report("Expected ':' after identifier.");
            break;

        } else advance();


        const std::string value_str = token.get_value();
        Identifier_value value = token.get_value();

        if ( not is_token( allowed_type )) {
            auto type_str = std::string( token.get_typestr( allowed_type ));

            std::ranges::transform( type_str, type_str.begin(),
                []( unsigned char c ){
                    return std::tolower(c);
                }
            );

            report( "Expected valid {} for '{}' but got '{}'.",
                type_str,
                identifier,
                value_str
            );

        } else if ( is_token( STRING ) and token.get_value().empty() ) {
            report(
                "The value for '{}' cannot be an empty string.",
                identifier
            );
            break;

        } else if ( is_token( VALID_NUMBER ) ) {
            auto valid_int32 = parse_int32( value_str );

            if ( not valid_int32 ) {
                report( "Invalid int32 value: '{}'.", value_str );
                break;

            } else value = valid_int32.value();

        }


        if ( not is_token( PATHS_BLOCK )) {
            advance(); // skip newline
            // TODO: Implement validation for the block that will contain
            // paths to include and exclude
        }


        if ( not is_token( NEWLINE )) {
            report("Expected newline but got '{}'", token.get_value());
            break;
        }

        /* if is valid <identifier> : <value> \n */
        identifiers_on_top.at( identifier ).second = value;
    }
}


void Parser::print_config() {
    for ( const auto &[identifier, values] : identifiers_on_top ) {
        std::visit( [&]( const auto& value ) {
            using T = std::decay_t< decltype( value )>;

            if constexpr (std::is_same_v<T, std::string>) {
                std::println("{:<14}: {}", identifier, value);

            } else if constexpr (std::is_same_v<T, int64_t>) {
                std::println("{:<14}: {}", identifier, value);

            } else if constexpr (std::is_same_v<T, std::shared_ptr<DirTree>>) {
                std::println("{}:", identifier);
                value->print_tree(11);
            }

        }, values.second);
    }
}


Parser::Parser ( Lexer &_lexer )
  : lexer { _lexer }
{
    parsing();
}
