// ---- LOCAL INCLUDES ----
//
#include "parsing/parser.hpp"
#include "parsing/token.hpp"
#include "parsing/tree.hpp"


// ---- EXTERNAL INCLUDES ----
//
#include <fmt/core.h>


// ---- STANDARD INCLUDES ----
//
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


bool Parser::advance() {
    this->token = lexer.get_next_token();
    return true;
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


        if ( not main_identifiers.contains( identifier) )
            return report_error( "Identifier Unknow '{}'", identifier);

        if ( is_duplicate( identifier ))
            return report_error("Duplicate identifier '{}'", identifier);

        if ( advance() and not is_token( ASSIGN ))
            return report_error("Expected ':' after identifier.");


        const auto parsed_value = validate_data_type( identifier );


        if ( not parsed_value.has_value() )
            return false;

        else advance();


        if ( not is_token( NEWLINE ) and not is_token( END_OF_FILE ))
          return report_error(
                "Expected newline but got '{}'",
                token.get_value()
            );


        main_identifiers.at( identifier ).second = parsed_value.value();
    }

    return true;
}


std::optional<Parser::ident_value_t>
Parser::validate_data_type( std::string_view identifier ) {
    using enum Token::Type;

    /* consume assign token */
    advance();

    ident_value_t raw_value   = token.get_value();
    std::string   value_str   = token.get_value();
    Token::Type   type_expect = main_identifiers.at(identifier).first;


    if ( type_expect == PATHS_BLOCK and is_token( NEWLINE )) {
        skip_empty_lines();

        if ( parse_paths_block( raw_value ))
            return raw_value;

        return {};
    }


    if ( is_token( STRING ) ) {
        if ( token.get_value().empty() ) {
            report_error( "The value for '{}' cannot be an empty string.",
                identifier
            );

            return {};
        }


        if ( type_expect == BASENAME or type_expect == EXISTING_PATH ) {
            // TODO: Check if the string is a valid path
        }

        return raw_value;
    }


    if ( is_token( VALID_NUMBER ) ) {
        auto valid_int32 = parse_int32( value_str );

        if ( not valid_int32 ) {
            report_error( "Invalid int32 value: '{}'.", value_str );
            return std::nullopt;
        }

        return valid_int32.value();
    }


    std::string typestr { token.get_typestr() };

    report_error( "Expected valid {} for '{}' but got '{}'.",
        get_lowercase( typestr ),
        identifier,
        value_str
    );

    return {};
}


bool Parser::parse_paths_block( ident_value_t &raw_value ) {
    using enum Token::Type;
    using NodeType = DirTree::NodeType;

    if ( not is_token( INDENT_SPACE ) and not is_token( INDENT_TAB ))
        return report_error(
            "Expected spaces or tabs after newline, but got '{}'",
            token.get_value()
        );


    /* Base indent size and type, inferred from the first block —
     * used as a reference
     */
    const size_t block_indent_size = token.get_value().length();
    const auto   indent_type       = token.get_type();


    /* Initialize to 1: root block is always indented once by design */
    std::size_t last_indent_level = 1;


    const auto root_name = std::get<std::string>(
        main_identifiers.at( "project_root" ).second
    );


    auto  tree_ptr = std::make_unique<DirTree>( root_name );
    auto &tree     = *tree_ptr;


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


        /* Save for precise error reporting */
        const auto indent_token = token;


        /* ignore empty lines (only comments) */
        if ( advance() and is_token( NEWLINE )) {
            skip_empty_lines();
            continue;
        };


        const std::size_t
            curr_indent_size  = indent_token.get_value().length(),
            curr_indent_level = curr_indent_size / block_indent_size,
            extra_indent      = curr_indent_size & (block_indent_size-1);


        if ( curr_indent_level > last_indent_level+1 )
            return report_error( indent_token,
                "Expected {} blocks of indentation, but got {}",
                last_indent_level+1,
                curr_indent_level
            );


        if ( extra_indent > 0 )
            return report_error( indent_token,
                "Extra indentation of {} character(s).",
                extra_indent
            );


        if ( last_node_type != NodeType::IS_DIRECTORY ) {
            if ( curr_indent_level > last_indent_level ) {
                return report_error( indent_token,
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


        const char type_indicator = token.get_value()[0];


        // TODO: add support for excluded paths
        if ( type_indicator == '-' ) {
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


        const Token path_token = token;
        advance();


        if ( is_token( SYMBOL ) and token.get_value() == "*" ) {
            if ( curr_node_type != NodeType::IS_DIRECTORY ) {
                return report_error(
                    "The '*' symbol is only valid for directories."
                );
            }

            path_select_all = true;
            advance();

        } else path_select_all = false;


        if ( not is_token( NEWLINE ))
            return report_error(
                "Expected newline after path, but got '{}'",
                token.get_value()
            );

        else skip_empty_lines();


        if ( curr_indent_level <= last_indent_level )
            tree.ascend_levels(last_indent_level - curr_indent_level + 1);


        const auto &path_name = path_token.get_value();
        const auto &curr_node = tree.get_curr_node  ();


        if ( curr_node.children.contains( path_name ) )
            return report_error( path_token,
                "Path '{}' duplicate.",
                path_name
            );


        (void)tree.add_child(
            path_name,
            curr_node_type
        );


        if ( curr_node_type == NodeType::IS_DIRECTORY )
            (void)tree.go_to_child( path_name);


        if ( path_select_all )
            tree.select_all_of(
                tree.get_curr_node()
            );


        last_indent_level = curr_indent_level;
        last_node_type    = curr_node_type   ;
    }

    raw_value = std::move(tree_ptr);
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


Parser::Parser ( Lexer &_lexer, ident_map_t &_main_identifiers )
  : lexer            { _lexer            },
    main_identifiers { _main_identifiers }
{
    if ( lexer.has_errors() ) {
        _has_errors = true;
        return;
    }

    parsing();
}
