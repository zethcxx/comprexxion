// --- My Includes:
#include "parsing/parser.hpp"
#include "parsing/token.hpp"
#include "parsing/tree.hpp"

// --- Standard Includes:
#include <charconv>
#include <filesystem>
#include <cstdint>
#include <string>
#include <unordered_set>


bool Parser::has_errors() const {
    return _has_errors;
}

bool Parser::is_token( const Token::Type &expected_token ) const {
    return ( token.get_type() == expected_token );
}


void Parser::advance() {
    this->token = lexer.get_next_token();
}


std::string Parser::get_current_dir_name( void ) {
    namespace fs = std::filesystem;

    const auto path_name = fs::absolute(".").parent_path().filename().string();

    if ( path_name.empty() )
        return "root_fs";
    else
        return path_name;
}


std::string Parser::get_current_dir_path( void ) {
    namespace fs = std::filesystem;

    return fs::absolute(".").parent_path().string();
}


std::string Parser::get_lowercase( std::string_view str ) {
    std::string lowercase;

    for (const int c : str) {
        lowercase.push_back(
            ( c == '_' ) ? ' ' : (char)std::tolower(c)
        );
    }

    return lowercase;
}



std::optional<std::int32_t>
Parser::parse_int32( std::string_view string ) const {
    std::int32_t result;

    const auto [ptr, ec] = std::from_chars(
        string.begin(),
        string.end(),
        result
    );

    if ( ec == std::errc() && ptr == string.end() )
        return result;

    return std::nullopt;
}


void Parser::skip_empty_lines() {
    while ( is_token( TOKEN::NEWLINE )) advance();
}


void Parser::parsing() {
    using enum Token::Type;

    std::unordered_set<std::string> identifiers_used;

    const auto is_duplicate = [&]( const std::string &identifier ) -> bool {
        if ( identifiers_used.contains( identifier ) ) return true;

        identifiers_used.insert( identifier );
        return false;
    };

    if ( is_token( BEGIN_OF_FILE )) advance();

    while ( not is_token( END_OF_FILE )) {

        skip_empty_lines();

        /* Stop if the file only had empty lines */
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

        } else if ( is_duplicate( identifier )) {
            report("Duplicate identifier '{}'", identifier);
            break;
        }

        advance();

        if ( not is_token( ASSIGN )) {
            report("Expected ':' after identifier.");
            break;

        } else advance();


        const auto parsed_value = validate_data_type( identifier );

        if ( parsed_value.has_value() )
            advance();
        else
            break;


        if ( not is_token( NEWLINE ) and not is_token( END_OF_FILE )) {
            report("Expected newline but got '{}'", token.get_value());
            break;
        }

        /* if is valid <identifier> : <value> \n */
        identifiers_on_top.at( identifier ).second = parsed_value.value();
    }
}


std::optional<Parser::Identifier_value>
Parser::validate_data_type( std::string_view identifier ) {
    using enum TOKEN;

    Identifier_value raw_value = token.get_value();
    std::string      value_str = token.get_value();
    Token::Type      type      = identifiers_on_top.at(identifier).first;

    if ( type == PATHS_BLOCK and is_token( NEWLINE )) {
        skip_empty_lines();

        if ( not is_token( INDENT_SPACE ) and not is_token( INDENT_TAB )) {
            report( "Expected spaces or tabs after newline, but got '{}'",
                    token.get_value()
            );

            return std::nullopt;
        }

        if ( not parse_paths_block( raw_value ) )
            return std::nullopt;

    } else if ( not is_token( type )) {
        std::string typestr { token.get_typestr() } ;
        report( "Expected valid {} for '{}' but got '{}'.",
                get_lowercase( typestr ),
                identifier,
                value_str
        );

        return std::nullopt;
    }


    if ( is_token( STRING ) and token.get_value().empty() ) {
        report( "The value for '{}' cannot be an empty string.",
                identifier
        );

        return std::nullopt;
    }


    if ( is_token( VALID_NUMBER ) ) {
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
    using enum TOKEN;

    const size_t block_indent_size = token.get_value().length();
    const auto   indent_type       = token.get_type();

    std::size_t before_indent_level = 1;

    const std::string root_name = std::get<std::string>(
        identifiers_on_top.at( "project_root" ).second
    );
    auto tree = std::make_shared<DirTree>( root_name );

    bool last_was_directory = false;
    while ( not is_token( END_OF_FILE )) {

        if ( is_token ( INDENT_MIXED )) {
            report( "Mixed spaces and tabs" );
            return false;
        }


        if ( not is_token( indent_type )) {
            report( "Expected {} after newline, but got '{}'",
                    get_lowercase( token.get_typestr()),
                    token.get_value()
            );

            return false;
        }


        const std::size_t
                indent_size  = token.get_value().length(),
                indent_level = indent_size / block_indent_size,
                extra_indent = indent_size & (block_indent_size-1);

        const auto previous_token = token;

        advance();

        const auto current_token = token;

        /* ignore empty lines (only comments) */
        if ( is_token( NEWLINE )) {
            skip_empty_lines();
            continue;
        };


        /* use previous token for error reporting */
        token = previous_token;

        if ( indent_level > before_indent_level+1 ) {
            report(
                "Expected {} blocks of indentation, but got {}",
                before_indent_level+1,
                indent_level
            );

        } else if ( extra_indent > 0 ) {
            report(
                "Extra indentation of {} character(s).",
                extra_indent
            );

        } else if ( not last_was_directory ) {
            if ( indent_level > before_indent_level ) {
                report(
                    "Expected {} blocks of indentation, but got {}",
                    before_indent_level,
                    indent_level
                );
            }
        }

        if ( has_errors() ) return false;

        /* restore token */
        token = current_token;

        if ( not is_token( PATH_INDICATOR )) {
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


        if ( not is_token( IDENTIFIER )) {
            report(
                "Expected identifier (f/d), but got '{}'",
                token.get_value()
            );
            return false;

        }

        const std::string path_identifier = token.get_value();

        if ( path_identifier != "f" and path_identifier != "d" ) {
            report(
                "Invalid identifier: '{}'",
                token.get_value()
            );
            return false;
        }

        bool is_directory = path_identifier == "d";
        last_was_directory = is_directory;
        advance();


        if ( not is_token( STRING )) {
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


        if ( not is_token( NEWLINE )) {
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
    if ( lexer.has_errors() ) {
        _has_errors = true;
        return;
    }

    parsing();
}
