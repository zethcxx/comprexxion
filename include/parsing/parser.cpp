// --- My Includes:
#include "parsing/parser.hpp"
#include "parsing/token.hpp"
#include "parsing/tree.hpp"

// --- External Includes:
#include <fmt/core.h>

// --- Standard Includes:
#include <charconv>
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
    while ( is_token( Token::Type::NEWLINE )) advance();
}


bool Parser::parsing() {
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
            return report_error(
                "Identifier expected, but got '{}'",
                token.get_value()
            );
        }


        const std::string identifier = token.get_value();


        if ( not main_identifiers.contains( identifier) ) {
            return report_error( "Identifier Unknow '{}'", identifier);


        } else if ( is_duplicate( identifier )) {
            return report_error("Duplicate identifier '{}'", identifier);
        }

        advance();

        if ( not is_token( ASSIGN )) {
            return report_error("Expected ':' after identifier.");

        } else advance();


        const auto parsed_value = validate_data_type( identifier );


        if ( not parsed_value.has_value() )
            return false;

        advance();


        if ( not is_token( NEWLINE ) and not is_token( END_OF_FILE )) {
            return report_error("Expected newline but got '{}'", token.get_value());

        }

        /* if is valid <identifier> : <value> \n */
        main_identifiers.at( identifier ).second = parsed_value.value();
    }

    return true;
}


std::optional<Parser::Identifier_value>
Parser::validate_data_type( std::string_view identifier ) {
    using enum Token::Type;


    Identifier_value raw_value = token.get_value();
    std::string      value_str = token.get_value();
    Token::Type      type      = main_identifiers.at(identifier).first;


    if ( type == PATHS_BLOCK and is_token( NEWLINE )) {
        skip_empty_lines();

        if ( not is_token( INDENT_SPACE ) and not is_token( INDENT_TAB )) {
            report_error( "Expected spaces or tabs after newline, but got '{}'",
                    token.get_value()
            );

            return std::nullopt;
        }

        if ( not parse_paths_block( raw_value ) )
            return std::nullopt;

    } else if ( not is_token( type )) {
        std::string typestr { token.get_typestr() } ;
        report_error( "Expected valid {} for '{}' but got '{}'.",
                get_lowercase( typestr ),
                identifier,
                value_str
        );

        return std::nullopt;
    }


    if ( is_token( STRING ) and token.get_value().empty() ) {
        report_error( "The value for '{}' cannot be an empty string.",
                identifier
        );

        return std::nullopt;
    }


    if ( is_token( VALID_NUMBER ) ) {
        auto valid_int32 = parse_int32( value_str );

        if ( not valid_int32 ) {
            report_error( "Invalid int32 value: '{}'.", value_str );
            return std::nullopt;
        }

        raw_value = valid_int32.value();
    }

    return raw_value;
}


bool Parser::parse_paths_block( Identifier_value &raw_value ) {
    using enum Token::Type;
    using NodeType = DirTree::NodeType;

    /* Base indent size and type, inferred from the first block —
     * used as a reference
     */
    const size_t block_indent_size = token.get_value().length();
    const auto   indent_type       = token.get_type();

    /* Ensure at least one indent block */
    std::size_t last_indent_level = 1;

    const std::string root_name = std::get<std::string>(
        main_identifiers.at( "project_root" ).second
    );
    auto tree = std::make_shared<DirTree>( root_name );


    NodeType last_node_type = NodeType::IS_FILE;
    bool path_select_all = false;


    while ( not is_token( END_OF_FILE )) {

        if ( is_token ( INDENT_MIXED ))
            return report_error( "Mixed spaces and tabs" );


        if ( not is_token( indent_type ))
            return report_error( "Expected {} after newline, but got '{}'",
                    get_lowercase( token.get_typestr()),
                    token.get_value()
            );


        const std::size_t
                curr_indent_size  = token.get_value().length(),
                curr_indent_level = curr_indent_size / block_indent_size,
                extra_indent      = curr_indent_size & (block_indent_size-1);

        /* Store token before advancing, to use it in accurate error messages
         */
        const auto &previous_token = token;

        advance();

        /* ignore empty lines (only comments) */
        if ( is_token( NEWLINE )) {
            skip_empty_lines();
            continue;
        };


        if ( curr_indent_level > last_indent_level+1 ) {
            return report_error(
                previous_token,
                "Expected {} blocks of indentation, but got {}",
                last_indent_level+1,
                curr_indent_level
            );

        } else if ( extra_indent > 0 ) {
            return report_error(
                previous_token,
                "Extra indentation of {} character(s).",
                extra_indent
            );

        } else if ( last_node_type != NodeType::IS_DIRECTORY ) {
            if ( curr_indent_level > last_indent_level ) {
                return report_error(
                    previous_token,
                    "Expected {} blocks of indentation, but got {}",
                    last_indent_level,
                    curr_indent_level
                );
            }
        }


        if ( not is_token( PATH_INDICATOR ))
            return report_error(
                "Expected path indicator (+/-), but got '{}'",
                token.get_value()
            );


        const char tye_indicator = token.get_value()[0];

        // TODO: add support for excluded paths
        if ( tye_indicator == '-' ) {
            return report_error(
                "Invalid path indicator: '{}'",
                token.get_value()
            );

        }

        /* reject '+' if previous directory used '*' */
        if ( curr_indent_level > last_indent_level and path_select_all )
            return report_error(
                "Redundant usage: '+' is not allowed within a block with '*'."
            );


        advance();


        if ( not is_token( IDENTIFIER ))
            return report_error(
                "Expected identifier (f/d), but got '{}'",
                token.get_value()
            );


        const std::string path_identifier = token.get_value();


        if ( path_identifier != "f" and path_identifier != "d" ) {
            return report_error(
                "Invalid identifier: '{}'",
                token.get_value()
            );
        }


        NodeType curr_node_type = path_identifier == "d"
            ? NodeType::IS_DIRECTORY
            : NodeType::IS_FILE;

        advance();


        if ( not is_token( STRING ))
            return report_error(
                "Expected string, but got '{}'",
                token.get_value()
            );

        else if ( token.get_value().empty() )
            return report_error( "The path cannot be an empty string." );


        const std::string path = token.get_value();
        advance();


        if ( is_token( SYMBOL ) and token.get_value() == "*" ) {
            if ( curr_node_type != NodeType::IS_DIRECTORY ) {
                return report_error(
                    "The '*' symbol can only be used in directories."
                );
            }

            path_select_all = true;

            /*
                TODO: Scan and select all items in current directory node

                const auto path = tree->get_curr_node();
                path.select_all();
            */

            advance();

        } else path_select_all = false;



        if ( not is_token( NEWLINE ))
            return report_error(
                "Expected newline after path, but got '{}'",
                token.get_value()
            );

        else skip_empty_lines();


        if ( curr_indent_level <= last_indent_level ) {
            size_t levels_up = last_indent_level - curr_indent_level;

            for ( size_t i = 0; i <= levels_up; i++ )
                (void)tree->go_to_parent();
        }



        (void)tree->add_child(
            path,
            curr_node_type
        );


        if ( curr_node_type == NodeType::IS_DIRECTORY )
            (void)tree->go_to_child( path );


        last_indent_level = curr_indent_level;
        last_node_type    = curr_node_type   ;
    }

    raw_value = tree;
    return true;
}


void Parser::print_config() {
    for ( const auto &[identifier, values] : main_identifiers ) {
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


Parser::Parser ( Lexer &_lexer, Identifiers_map &_main_identifiers )
  : lexer            { _lexer            },
    main_identifiers { _main_identifiers }
{
    if ( lexer.has_errors() ) {
        _has_errors = true;
        return;
    }

    parsing();
}
