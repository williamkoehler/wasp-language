#include "wasp_lexer.g.h"

int64_t wasp_string_to_integer(unsigned char* start, unsigned char* end, int64_t base)
{
    int64_t result = 0;

    for (; start < end; start++)
    {
        result *= base;

        if (*start >= 'a' && *start <= 'f')
            result += (*start - 'a');
        else if (*start >= 'A' && *start <= 'F')
            result += (*start - 'A');
        else
            result += (*start - '0');
    }

    return result;
}

bool wasp_create_lexer(wasp_lexer* lexer, unsigned char* text, size_t text_length)
{
    assert(lexer != NULL);

    if (text[text_length] != '\0')
        return false;

    lexer->start_position = text;
    lexer->end_position = text + text_length;
    lexer->current_position = lexer->start_position;

    // Prime lexer
    lexer->token[WASP_LAST_TOKEN].type = WASP_UNKNOWN_TOKEN;
    lexer->token[WASP_LAST_TOKEN].line_number = 1;

    for (size_t index = 0; index < WASP_LOOKAHEAD_SIZE; index++)
        wasp_lexer_advance(lexer);

    return true;
}

void wasp_lexer_advance(wasp_lexer* lexer)
{
    // Shift token
    memcpy(&lexer->token[0], &lexer->token[1], sizeof(wasp_token) * (WASP_LOOKAHEAD_SIZE - 1));

    // Read next token
    if (lexer->token[WASP_LAST_TOKEN].type != WASP_EOF_TOKEN)
    {
        unsigned char* current_position;
        unsigned char* token_start_position;
        unsigned char* token_end_position;

        size_t state_id, last_final_state_id;
        uint8_t state_flags;

        do
        {
            current_position = lexer->current_position;
            token_start_position = current_position;

            state_id = 1;
            last_final_state_id = 0;

            do
            {
                uint8_t c = *current_position++;
                state_id = wasp_state_transitions[((size_t)(state_id << 8) | (size_t)c)];
                // printf("State: %s\n", wasp_state_names[state_id]);
                state_flags = wasp_state_flags[state_id];

                if (state_flags & 0x1)
                {
                    token_end_position = current_position;
                    last_final_state_id = state_id;
                }

                if (c == '\n')
                    lexer->token[WASP_LAST_TOKEN].line_number++;
            }
            while (state_flags & 0x2);

            // Set position to token end position
            lexer->current_position = token_end_position;

            // Get and check token
            lexer->token[WASP_LAST_TOKEN].type = wasp_state_token[last_final_state_id];
        }
        while (lexer->token[WASP_LAST_TOKEN].type == WASP_WHITESPACE_SPECIAL_TOKEN);

        lexer->token[WASP_LAST_TOKEN].text.data = token_start_position;
        lexer->token[WASP_LAST_TOKEN].text.data_length = (size_t)(token_end_position - token_start_position);

        // printf("%s: '%.*s'\n", wasp_lexer_get_token_name(state->token_2.type),
        //    (int)(token_end_position - token_start_position), token_start_position);

        switch (lexer->token[WASP_LAST_TOKEN].type)
        {
        case WASP_IDENTIFIER_TOKEN:
            lexer->token[WASP_LAST_TOKEN].value.string = lexer->token[WASP_LAST_TOKEN].text;
            break;
        case WASP_INTEGER_TOKEN:
            lexer->token[WASP_LAST_TOKEN].value.integer =
                wasp_string_to_integer(token_start_position, token_end_position, 10);
            break;
        case WASP_BIN_INTEGER_SPECIAL_TOKEN:
            lexer->token[WASP_LAST_TOKEN].type = WASP_INTEGER_TOKEN;
            lexer->token[WASP_LAST_TOKEN].value.integer =
                wasp_string_to_integer(token_start_position + 2, token_end_position, 2);
            break;
        case WASP_OCT_INTEGER_SPECIAL_TOKEN:
            lexer->token[WASP_LAST_TOKEN].type = WASP_INTEGER_TOKEN;
            lexer->token[WASP_LAST_TOKEN].value.integer =
                wasp_string_to_integer(token_start_position + 1, token_end_position, 8);
            break;
        case WASP_HEX_INTEGER_SPECIAL_TOKEN:
            lexer->token[WASP_LAST_TOKEN].type = WASP_INTEGER_TOKEN;
            lexer->token[WASP_LAST_TOKEN].value.integer =
                wasp_string_to_integer(token_start_position + 2, token_end_position, 16);
            break;
        case WASP_NUMBER_TOKEN:
            lexer->token[WASP_LAST_TOKEN].value.number = strtod((const char*)token_start_position, NULL);
            break;
        case WASP_STRING_LITERAL_TOKEN:
            lexer->token[WASP_LAST_TOKEN].value.string.data = lexer->token[WASP_LAST_TOKEN].text.data + 1;
            lexer->token[WASP_LAST_TOKEN].value.string.data_length = lexer->token[WASP_LAST_TOKEN].text.data_length - 2;
            break;
        default:
            break;
        }
    }
}

static const char* wasp_token_names[] = {
    "invalid: unknown token",
    "end of file",

    ".",
    ";",
    ":",
    ",",
    "_",
    "(",
    ")",
    "{",
    "}",
    "[",
    "]",
    "<",
    ">",

    "=",
    "==",
    "+",
    "+=",
    "++",
    "-",
    "-=",
    "--",
    "*",
    "*=",
    "/",
    "/=",
    "%",
    "%=",
    "!",
    "!=",
    "<=",
    "<<",
    ">=",
    ">>",
    "~",
    "&",
    "&&",
    "|",
    "||",
    "^",
    "^^",
    "#",
    "?",
    "?=",
    "??",
    "$",
    "->",

    "identifier",
    "integer",
    "number",
    "string literal",

    "null",
    "true",
    "false",
    "new",
    "struct",
    "ctor",
    "dtor",
    "func",
    "operator",
    "if",
    "else",
    "while",
    "do",
    "for",
    "return",
    "const",
    "void",
    "invalid: token count",

    "whitespace",

    "bin integer",
    "oct integer",
    "hex integer",
};

void wasp_lexer_print_token(wasp_token token)
{
    printf("%s (line %zu)\n", wasp_token_names[token.type], token.line_number);
}