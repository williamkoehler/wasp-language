#pragma once
#include "wasp_common.h"

/// @brief Wasp token type
///
typedef enum wasp_token_type : uint8_t
{
    WASP_UNKNOWN_TOKEN = 0,
    WASP_EOF_TOKEN, // End of file

    WASP_DOT_TOKEN,              // .
    WASP_SEMICOLON_TOKEN,        // ;
    WASP_COLON_TOKEN,            // :
    WASP_COMMA_TOKEN,            // ,
    WASP_UNDERSCORE_TOKEN,       // _
    WASP_L_PARENTHESES_TOKEN,    // (
    WASP_R_PARENTHESES_TOKEN,    // )
    WASP_L_CURLY_BRACKET_TOKEN,  // {
    WASP_R_CURLY_BRACKET_TOKEN,  // }
    WASP_L_BRACKET_TOKEN,        // [
    WASP_R_BRACKET_TOKEN,        // ]
    WASP_L_ANGLED_BRACKET_TOKEN, // <
    WASP_R_ANGLED_BRACKET_TOKEN, // >

    WASP_EQUALS_TOKEN,                         // =
    WASP_DOUBLE_EQUALS_TOKEN,                  // ==
    WASP_PLUS_TOKEN,                           // +
    WASP_PLUS_EQUALS_TOKEN,                    // +=
    WASP_DOUBLE_PLUS_TOKEN,                    // ++
    WASP_MINUS_TOKEN,                          // -
    WASP_MINUS_EQUALS_TOKEN,                   // -=
    WASP_DOUBLE_MINUS_TOKEN,                   // --
    WASP_ASTERISK_TOKEN,                       // *
    WASP_ASTERISK_EQUALS_TOKEN,                // *=
    WASP_SLASH_TOKEN,                          // /
    WASP_SLASH_EQUALS_TOKEN,                   // /=
    WASP_PERCENT_TOKEN,                        // %
    WASP_PERCENT_EQUALS_TOKEN,                 // %=
    WASP_EXCLAMATION_MARK_TOKEN,               // !
    WASP_EXCLAMATION_MARK_EQUALS_TOKEN,        // !=
    WASP_L_ANGLED_BRACKET_EQUALS_TOKEN,        // <=
    WASP_DOUBLE_L_ANGLED_BRACKET_TOKEN,        // <<
    WASP_R_ANGLED_BRACKET_EQUALS_TOKEN,        // >=
    WASP_DOUBLE_R_ANGLED_BRACKET_TOKEN,        // >>
    WASP_TILDE_TOKEN,                          // ~
    WASP_AMPERSAND_TOKEN,                      // &
    WASP_DOUBLE_AMPERSAND_TOKEN,               // &&
    WASP_PIPE_TOKEN,                           // |
    WASP_DOUBLE_PIPE_TOKEN,                    // ||
    WASP_CARET_TOKEN,                          // ^
    WASP_DOUBLE_CARET_TOKEN,                   // ^^
    WASP_HASH_TOKEN,                           // #
    WASP_QUESTION_MARK_TOKEN,                  // ?
    WASP_QUESTION_MARK_ASSIGN_TOKEN,           // ?=
    WASP_DOUBLE_QUESTION_MARK_TOKEN,           // ??
    WASP_DOLLAR_TOKEN,                         // $
    WASP_ARROW_TOKEN,                          // ->

    WASP_IDENTIFIER_TOKEN,     // some_identifier
    WASP_INTEGER_TOKEN,        // 0
    WASP_NUMBER_TOKEN,         // 0.0
    WASP_STRING_LITERAL_TOKEN, // "some string"

    // Keywords
    WASP_NULL_KEYWORD_TOKEN,     // null
    WASP_TRUE_KEYWORD_TOKEN,     // true
    WASP_FALSE_KEYWORD_TOKEN,    // false
    WASP_NEW_KEYWORD_TOKEN,      // new
    WASP_STRUCT_KEYWORD_TOKEN,   // struct
    WASP_CTOR_KEYWORD_TOKEN,     // ctor
    WASP_DTOR_KEYWORD_TOKEN,     // dtor
    WASP_FUNCTION_KEYWORD_TOKEN, // func
    WASP_OPERATOR_KEYWORD_TOKEN, // operator
    WASP_IF_KEYWORD_TOKEN,       // if
    WASP_ELSE_KEYWORD_TOKEN,     // else
    WASP_WHILE_KEYWORD_TOKEN,    // while
    WASP_DO_KEYWORD_TOKEN,       // do
    WASP_FOR_KEYWORD_TOKEN,      // for
    WASP_BREAK_KEYWORD_TOKEN,   // break
    WASP_CONTINUE_KEYWORD_TOKEN,   // continue
    WASP_RETURN_KEYWORD_TOKEN,   // return
    WASP_CONST_KEYWORD_TOKEN,    // const
    WASP_VOID_KEYWORD_TOKEN,     // void
    WASP_TOKEN_COUNT,

    WASP_WHITESPACE_SPECIAL_TOKEN,

    WASP_BIN_INTEGER_SPECIAL_TOKEN, // 0b01001011
    WASP_OCT_INTEGER_SPECIAL_TOKEN, // 0113
    WASP_HEX_INTEGER_SPECIAL_TOKEN, // 0x4B
} wasp_token_type;

// Token alternatives
#define WASP_ASSIGN_TOKEN WASP_EQUALS_TOKEN
#define WASP_ADDITION_TOKEN WASP_PLUS_TOKEN
#define WASP_ADDITION_ASSIGN_TOKEN WASP_PLUS_EQUALS_TOKEN
#define WASP_INCREMENT_TOKEN WASP_DOUBLE_PLUS_TOKEN
#define WASP_SUBTRACTION_TOKEN WASP_MINUS_TOKEN
#define WASP_SUBTRACTION_ASSIGN_TOKEN WASP_MINUS_EQUALS_TOKEN
#define WASP_DECREMENT_TOKEN WASP_DOUBLE_MINUS_TOKEN
#define WASP_MULTIPLICATION_TOKEN WASP_ASTERISK_TOKEN
#define WASP_MULTIPLICATION_ASSIGN_TOKEN WASP_ASTERISK_EQUALS_TOKEN
#define WASP_DIVISION_TOKEN WASP_SLASH_TOKEN
#define WASP_DIVISION_ASSIGN_TOKEN WASP_SLASH_EQUALS_TOKEN
#define WASP_MODULO_TOKEN WASP_PERCENT_TOKEN
#define WASP_MODULO_ASSIGN_TOKEN WASP_PERCENT_EQUALS_TOKEN
#define WASP_EQUAL_TOKEN WASP_DOUBLE_EQUALS_TOKEN
#define WASP_NOT_EQUAL_TOKEN WASP_EXCLAMATION_MARK_EQUALS_TOKEN
#define WASP_LESS_TOKEN WASP_L_ANGLED_BRACKET_TOKEN
#define WASP_LESS_EQUAL_TOKEN WASP_L_ANGLED_BRACKET_EQUALS_TOKEN
#define WASP_GREATER_TOKEN WASP_R_ANGLED_BRACKET_TOKEN
#define WASP_GREATER_EQUAL_TOKEN WASP_R_ANGLED_BRACKET_EQUALS_TOKEN
#define WASP_LOGICAL_AND_TOKEN WASP_DOUBLE_AMPERSAND_TOKEN
#define WASP_LOGICAL_OR_TOKEN WASP_DOUBLE_PIPE_TOKEN
#define WASP_LOGICAL_XOR_TOKEN WASP_DOUBLE_CARET_TOKEN
#define WASP_BITWISE_AND_TOKEN WASP_AMPERSAND_TOKEN
#define WASP_BITWISE_OR_TOKEN WASP_PIPE_TOKEN
#define WASP_BITWISE_XOR_TOKEN WASP_CARET_TOKEN
#define WASP_LEFT_SHIFT_TOKEN WASP_DOUBLE_L_ANGLED_BRACKET_TOKEN
#define WASP_RIGHT_SHIFT_TOKEN WASP_DOUBLE_R_ANGLED_BRACKET_TOKEN

/// @brief Wasp token
///
typedef struct wasp_token
{
    wasp_token_type type;
    wasp_string text;

    size_t line_number;

    union
    {
        wasp_string string;
        int64_t integer;
        double number;
    } value;
} wasp_token;

#define WASP_LOOKAHEAD_SIZE 6
#define WASP_LAST_TOKEN (WASP_LOOKAHEAD_SIZE - 1)

typedef struct wasp_lexer
{
    unsigned char* start_position;
    unsigned char* end_position;

    unsigned char* current_position;

    wasp_token token[WASP_LOOKAHEAD_SIZE];
} wasp_lexer;

bool wasp_create_lexer(wasp_lexer* lexer, unsigned char* text, size_t text_length);

void wasp_lexer_advance(wasp_lexer* lexer);
void wasp_lexer_advance_n(wasp_lexer* lexer, size_t n);

void wasp_lexer_print_token(wasp_token token);