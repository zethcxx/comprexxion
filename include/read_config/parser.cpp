#include "parser.hpp"
#include "read_config/tree.hpp"
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

std::string Parser::type_str_lowercase( const TOKEN &type ) {
    auto type_str = std::string( token.get_typestr( type ));

    /* convert type_str to lowercase */
    std::transform( type_str.begin(), type_str.end(), type_str.begin(),
        []( unsigned char c ){
            if ( c == '_' ) return int(' ');
            return std::tolower(c);
        }
    );

    return type_str;
}


void Parser::skip_empty_lines() {
    while ( is_token( TOKEN::NEWLINE )) advance();
}


void Parser::parsing() {
    std::unordered_set<std::string> identifiers_used;

    auto is_duplicate = [&]( const std::string &identifier ) -> bool {
        if ( identifiers_used.find( identifier ) != identifiers_used.end() )
            return true;

        identifiers_used.insert( identifier );
        return false;
    };

    if ( is_token( TOKEN::BEGIN_OF_FILE )) advance();

    while ( not is_token( TOKEN::END_OF_FILE )) {

        skip_empty_lines();

        /* Stop if the file only had empty lines */
        if ( is_token( TOKEN::END_OF_FILE )) break;


        if ( not is_token( TOKEN::IDENTIFIER )) {
            report(
                "Identifier expected, but got '{}'",
                token.get_value()
            );
            break;
        }


        const std::string identifier = token.get_value();


        if ( identifiers_on_top.find( identifier) == identifiers_on_top.end() ) {
            report( "Identifier Unknow '{}'", identifier);
            break;
        }


        if ( is_duplicate( identifier )) {
            report("Duplicate identifier '{}'", identifier);
            break;

        } else advance();


        if ( not is_token( TOKEN::ASSIGN )) {
            report("Expected ':' after identifier.");
            break;

        } else advance();


        const auto  parsed_value = validate_data_type( identifier );

        if ( not parsed_value.has_value() ) break;
        else advance();


        if ( not is_token( TOKEN::NEWLINE ) and not is_token( TOKEN::END_OF_FILE )) {
            report("Expected newline but got '{}'", token.get_value());
            break;
        }

        /* if is valid <identifier> : <value> \n */
        identifiers_on_top.at( identifier ).second = parsed_value.value();
    }
}


std::optional<Parser::Identifier_value>
Parser::validate_data_type( const std::string &identifier ) {

    Identifier_value raw_value   = token.get_value();
    std::string      value_str   = token.get_value();
    Token::Type      type        = identifiers_on_top.at(identifier).first;

    if ( type == TOKEN::PATHS_BLOCK and is_token( TOKEN::NEWLINE )) {
        skip_empty_lines();

        if ( not is_token( TOKEN::INDENT_SPACE ) and not is_token( TOKEN::INDENT_TAB )) {
            report( "Expected spaces or tabs after newline, but got '{}'",
                    token.get_value()
            );

            return std::nullopt;
        }

        bool success = parse_paths_block( raw_value );

        if ( not success )
            return std::nullopt;

    } else if ( not is_token( type )) {
        report( "Expected valid {} for '{}' but got '{}'.",
                type_str_lowercase( type ),
                identifier,
                value_str
        );

        return std::nullopt;

    }

    if ( is_token( TOKEN::STRING ) and token.get_value().empty() ) {
        report( "The value for '{}' cannot be an empty string.",
                identifier
        );

        return std::nullopt;
    }

    if ( is_token( TOKEN::VALID_NUMBER ) ) {
        auto valid_int32 = parse_int32( value_str );

        if ( not valid_int32 ) {
            report( "Invalid int32 value: '{}'.", value_str );
            return std::nullopt;
        }

        raw_value = valid_int32.value();
    }

    return raw_value;
}


bool Parser::parse_paths_block( Identifier_value &raw_value ) {
    const size_t block_indent_size = token.get_value().length();
    const auto   indent_type       = token.get_type();

    std::size_t before_indent_level = 1;

    std::string root_name = std::get<std::string>(
        identifiers_on_top.at( "project_root" ).second
    );
    auto tree = std::make_shared<DirTree>( root_name );

    bool last_was_directory = false;
    while ( not is_token( TOKEN::END_OF_FILE )) {

        if ( is_token ( TOKEN::INDENT_MIXED )) {
            report( "Mixed spaces and tabs" );
            return false;
        }

        if ( not is_token( indent_type )) {
            report( "Expected {} after newline, but got '{}'",
                    type_str_lowercase( indent_type ),
                    token.get_value()
            );
            return false;

        }

        const std::size_t
              indent_size  = token.get_value().length(),
              indent_level = indent_size / block_indent_size,
              extra_indent = indent_size % block_indent_size;

        const auto before_token = token;
        advance();
        const auto current_token = token;

        /* ignore empty lines */
        if ( is_token( TOKEN::NEWLINE )) {
            skip_empty_lines();
            continue;
        };


        token = before_token;
        if ( indent_level > before_indent_level+1 ) {
            report(
                "Expected {} blocks of indentation, but got {}",
                before_indent_level+1,
                indent_level
            );
            return false;

        } else if ( extra_indent > 0 ) {
            report(
                "Extra indentation of {} character(s).",
                extra_indent
            );
            return false;

        } else if ( not last_was_directory ) {
            if ( indent_level > before_indent_level ) {
                report(
                    "Expected {} blocks of indentation, but got {}",
                    before_indent_level,
                    indent_level
                );
                return false;
            }
        }

        token = current_token;


        if ( not is_token( TOKEN::PATH_INDICATOR )) {
            report(
                "Expected path indicator (+/-), but got '{}'",
                token.get_value()
            );
            return false;

        }

        const char tye_indicator = token.get_value()[0];

        // TODO: add support for excluded paths
        if ( tye_indicator == '-' ) {
            report(
                "Invalid path indicator: '{}'",
                token.get_value()
            );
            return false;
        }

        advance();


        if ( not is_token( TOKEN::IDENTIFIER )) {
            report(
                "Expected identifier (f/d), but got '{}'",
                token.get_value()
            );
            return false;

        }

        bool is_directory = token.get_value() == "d";
        last_was_directory = is_directory;
        advance();


        if ( not is_token( TOKEN::STRING )) {
            report(
                "Expected string, but got '{}'",
                token.get_value()
            );
            return false;

        } else if ( token.get_value().empty() ) {
            report( "The path cannot be an empty string." );
            return false;
        }

        const std::string path = token.get_value();
        advance();


        if ( not is_token( TOKEN::NEWLINE )) {
            report(
                "Expected newline after path, but got '{}'",
                token.get_value()
            );
            return false;

        } else skip_empty_lines();


        if ( indent_level < before_indent_level )
            (void)tree->go_to_parent();


        (void)tree->add_child(
            path,
            is_directory
        );

        if ( is_directory ) {
            (void)tree->go_to_child( path );
        }

        before_indent_level = indent_level;
    }

    raw_value = tree;
    return true;
}


void Parser::print_config() {
    for ( const auto &pair : identifiers_on_top ) {
        const auto &identifier = pair.first;
        const auto &values     = pair.second;

        std::visit( [&]( const auto& value ) {
            using T = std::decay_t< decltype( value )>;

            if constexpr (std::is_same_v<T, std::string>) {
                fmt::println("{:<14}: {}", identifier, value);

            } else if constexpr (std::is_same_v<T, int64_t>) {
                fmt::println("{:<14}: {}", identifier, value);

            } else if constexpr (std::is_same_v<T, std::shared_ptr<DirTree>>) {
                fmt::println("{}:", identifier);
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
