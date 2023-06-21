#include "wasp_compiler.h"
#include "wasp_ast_decl.h"
#include "wasp_ast_expr.h"
#include "wasp_ast_type.h"

bool wasp_create_compiler(wasp_compiler* compiler, unsigned char* text, size_t text_length)
{
    assert(compiler != NULL);

    // Create logger
    wasp_create_default_logger(&compiler->logger);
    compiler->error_count = 0;

    // Create lexer
    if (!wasp_create_lexer(&compiler->lexer, text, text_length))
    {
        compiler->logger.err("Failed to create lexer.");
        goto error;
    }

    // Create lazy allocator
    if (!wasp_create_lazy_allocator(&compiler->allocator))
    {
        compiler->logger.err("Failed to create lazy allocator.");
        goto error;
    }

    return true;

error:
    wasp_destroy_lazy_allocator(&compiler->allocator);
    return false;
}

bool wasp_compiler_set_allocation_jmp(wasp_compiler* compiler)
{
    return setjmp(compiler->allocation_jmp_buf) == 0;
}

void wasp_compiler_allocation_error(wasp_compiler* compiler, const char* type)
{
    char buffer[2048];

    sprintf(buffer, "Failed to allocate %s", type);

    compiler->logger.err(buffer);

    longjmp(compiler->allocation_jmp_buf, -1);
}

void wasp_compiler_error(wasp_compiler* compiler, const char* format, ...)
{
    char buffer[2048];

    va_list va_args;
    va_start(va_args, format);

    int p = vsprintf(buffer, format, va_args);

    va_end(va_args);

    compiler->logger.err(buffer);

    compiler->error_count++;
}

wasp_ast_type_list_item* create_ast_type_list_item(wasp_compiler* compiler)
{
    wasp_ast_type_list_item* item = wasp_lazy_alloc(&compiler->allocator, sizeof(wasp_ast_type_list_item));
    if (item == NULL)
        wasp_compiler_allocation_error(compiler, "ast type list item");

    return item;
}

wasp_ast_expr* create_ast_expr(wasp_compiler* compiler, wasp_ast_expr_type type)
{
    wasp_ast_expr* node = wasp_lazy_alloc(&compiler->allocator, sizeof(wasp_ast_expr));
    if (node == NULL)
        wasp_compiler_allocation_error(compiler, "ast expression");

    node->type = type;

    return node;
}

wasp_ast_expr_list_item* create_ast_expr_list_item(wasp_compiler* compiler, wasp_ast_expr* expr)
{
    wasp_ast_expr_list_item* item = wasp_lazy_alloc(&compiler->allocator, sizeof(wasp_ast_expr_list_item));
    if (item == NULL)
        wasp_compiler_allocation_error(compiler, "ast expression list item");

    item->expr = expr;

    return item;
}

// -------- Type Parsing --------

bool parse_type_generics(wasp_compiler* compiler, wasp_ast_type_list_item** result)
{
    assert(result != NULL);

    wasp_lexer_advance(&compiler->lexer); // Skip left angled bracket

    wasp_ast_type_list_item* last_item = NULL;

    while (compiler->lexer.token[0].type != WASP_R_BRACKET_TOKEN)
    {
        wasp_ast_type type;

        // Parse value
        if (!wasp_compiler_parse_type(compiler, &type))
        {
            wasp_compiler_error(compiler, "Expected type defintion as type generics item.");
            return false;
        }

        // Add generics item
        {
            wasp_ast_type_list_item* new_item = create_ast_type_list_item(compiler);
            new_item->type = type;

            if (last_item != NULL)
                last_item->next_item = new_item;
            else
                *result = new_item;

            last_item = new_item;
        }

        // Parse comma (or end bracket)
        {
            if (compiler->lexer.token[0].type == WASP_COMMA_TOKEN)
                wasp_lexer_advance(&compiler->lexer); // Skip comma
            else if (compiler->lexer.token[0].type != WASP_R_BRACKET_TOKEN)
            {
                wasp_compiler_error(compiler, "Expected ',' or ']' after type generics item.");

                // TODO Handle error
                return false;
            }
        }
    }

    if (last_item != NULL)
        last_item->next_item = NULL; // End linked list

    wasp_lexer_advance(&compiler->lexer); // Skip right bracket

    return true;
}

bool wasp_compiler_parse_type(wasp_compiler* compiler, wasp_ast_type* result)
{
    assert(result != NULL);

    result->is_reference = false;
    result->generics.first_item = NULL;

    if (compiler->lexer.token[0].type != WASP_IDENTIFIER_TOKEN)
    {
        // TODO Handle Error
    }

    result->name = compiler->lexer.token[0].text;

    wasp_lexer_advance(&compiler->lexer); // Skip identifier

    // Parse generics
    if (compiler->lexer.token[0].type == WASP_L_BRACKET_TOKEN)
    {
        if (!parse_type_generics(compiler, &result->generics.first_item))
            return false;
    }

    // Parse reference
    if (compiler->lexer.token[0].type == WASP_AMPERSAND_TOKEN)
    {
        wasp_lexer_advance(&compiler->lexer); // Skip &

        result->is_reference = true;
    }

    // Handle misplaced generics
    if (compiler->lexer.token[0].type == WASP_L_BRACKET_TOKEN)
    {
        wasp_compiler_error(compiler, "Type reference indicator should be placed after generics definition.");

        if (!parse_type_generics(compiler, &result->generics.first_item))
            return false;
    }

    return true;
}

// -------- Decl Parsing --------

bool wasp_compiler_parse_decl(wasp_compiler* compiler, wasp_ast_decl* result)
{
    assert(result != NULL);

    // Parse type
    if (!wasp_compiler_parse_type(compiler, &result->type))
    {
        // TODO Handle error
        return false;
    }

    // Parse name
    if (compiler->lexer.token[0].type != WASP_IDENTIFIER_TOKEN)
    {
        // TODO Handle error
        return false;
    }

    result->name = compiler->lexer.token[0].text;

    wasp_lexer_advance(&compiler->lexer); // Skip identifier

    return true;
}

// -------- Expression Parsing --------

const uint8_t OP_PRECEDENCE[] = {
    0,

    0, // unary &a
    0, // unary +a
    0, // unary -a
    0, // unary ++a
    0, // unary a++
    0, // unary --a
    0, // unary a--
    0, // unary !a
    0, // unary ~a

    1,  // binary a . b
    3,  // binary a & b
    4,  // binary a ^ b
    5,  // binary a | b
    6,  // binary a << b
    6,  // binary a << b
    7,  // binary a * b
    7,  // binary a / b
    7,  // binary a % b
    8,  // binary a + b
    8,  // binary a - b
    9,  // binary a > b
    9,  // binary a >= b
    9,  // binary a < b
    9,  // binary a <= b
    10, // binary a == b
    10, // binary a != b
    11, // binary a && b
    12, // binary a ^^ b
    14, // binary a || b
    15, // binary a == b
    15, // binary a *= b
    15, // binary a /= b
    15, // binary a %= b
    15, // binary a += b
    15, // binary a -= b
    15, // binary a &&= b
    15, // binary a ^^= b
    15, // binary a ||= b
    15, // binary a &= b
    15, // binary a ^= b
    15, // binary a |= b
    15, // binary a <<= b
    15, // binary a >>= b

    14, // ternary a ? b : c

    2, // nary [a,b,c,d,...]
};

void parse_string_literal(wasp_compiler* compiler, wasp_string* result)
{
    result->data = wasp_lazy_alloc(&compiler->allocator, compiler->lexer.token[0].value.string.data_length);
    if (result->data == NULL)
        wasp_compiler_allocation_error(compiler, "string");

    const unsigned char* string_literal_start = compiler->lexer.token[0].value.string.data;
    const unsigned char* string_literal_end = string_literal_start + compiler->lexer.token[0].value.string.data_length;

    size_t length = 0;
    for (const unsigned char* c = string_literal_start; c < string_literal_end; c++)
    {
        unsigned char character = *c;
        if (character == '\\')
        {
            character = ' ';

            c++;
            switch (*c)
            {
            case '\\':
                character = '\\';
                break;
            case '\a':
                character = '\a';
                break;
            case '\b':
                character = '\b';
                break;
            case '\f':
                character = '\f';
                break;
            case '\n':
                character = '\n';
                break;
            case '\r':
                character = '\r';
                break;
            case '\t':
                character = '\t';
                break;
            case '\v':
                character = '\v';
                break;
            case '\'':
                character = '\'';
                break;
            case '"':
                character = '"';
                break;
            }
        }

        result->data[length++] = character;
    }

    result->data_length = length;
}

wasp_ast_expr_operator get_operator(wasp_token token)
{
    switch (token.type)
    {
        // ---- Binary operator ----
    case WASP_DOT_TOKEN:
        return WASP_BINARY_EXPR_MEMBER_ACCESS_OP;
    case WASP_BITWISE_AND_TOKEN:
        return WASP_BINARY_EXPR_BITWISE_AND_OP;
    case WASP_BITWISE_XOR_TOKEN:
        return WASP_BINARY_EXPR_BITWISE_XOR_OP;
    case WASP_BITWISE_OR_TOKEN:
        return WASP_BINARY_EXPR_BITWISE_OR_OP;
    case WASP_LEFT_SHIFT_TOKEN:
        return WASP_BINARY_EXPR_BITWISE_LEFT_SHIFT_OP;
    case WASP_RIGHT_SHIFT_TOKEN:
        return WASP_BINARY_EXPR_BITWISE_RIGHT_SHIFT_OP;
    case WASP_MULTIPLICATION_TOKEN:
        return WASP_BINARY_EXPR_MULTIPLICATION_OP;
    case WASP_DIVISION_TOKEN:
        return WASP_BINARY_EXPR_DIVISION_OP;
    case WASP_MODULO_TOKEN:
        return WASP_BINARY_EXPR_MODULUS_OP;
    case WASP_ADDITION_TOKEN:
        return WASP_BINARY_EXPR_ADDITION_OP;
    case WASP_SUBTRACTION_TOKEN:
        return WASP_BINARY_EXPR_SUBTRACTION_OP;
    case WASP_GREATER_TOKEN:
        return WASP_BINARY_EXPR_GREATER_OP;
    case WASP_GREATER_EQUAL_TOKEN:
        return WASP_BINARY_EXPR_GREATER_EQUAL_OP;
    case WASP_LESS_TOKEN:
        return WASP_BINARY_EXPR_LESSER_OP;
    case WASP_LESS_EQUAL_TOKEN:
        return WASP_BINARY_EXPR_LESSER_OP;
    case WASP_EQUAL_TOKEN:
        return WASP_BINARY_EXPR_EQUAL_OP;
    case WASP_NOT_EQUAL_TOKEN:
        return WASP_BINARY_EXPR_NOT_EQUAL_OP;
    case WASP_LOGICAL_AND_TOKEN:
        return WASP_BINARY_EXPR_LOGICAL_AND_OP;
    case WASP_LOGICAL_XOR_TOKEN:
        return WASP_BINARY_EXPR_LOGICAL_XOR_OP;
    case WASP_LOGICAL_OR_TOKEN:
        return WASP_BINARY_EXPR_LOGICAL_OR_OP;

    case WASP_ASSIGN_TOKEN:
        return WASP_BINARY_ASSIGNMENT_OP;
    case WASP_MULTIPLICATION_ASSIGN_TOKEN:
        return WASP_BINARY_EXPR_MULTIPLICATION_ASSIGNMENT_OP;
    case WASP_DIVISION_ASSIGN_TOKEN:
        return WASP_BINARY_EXPR_DIVISION_ASSIGNMENT_OP;
    case WASP_MODULO_ASSIGN_TOKEN:
        return WASP_BINARY_EXPR_MODULUS_ASSIGNMENT_OP;
    case WASP_ADDITION_ASSIGN_TOKEN:
        return WASP_BINARY_EXPR_ADDITION_ASSIGNMENT_OP;
    case WASP_SUBTRACTION_ASSIGN_TOKEN:
        return WASP_BINARY_EXPR_SUBTRACTION_ASSIGNMENT_OP;

    // ---- Ternary operator ----
    case WASP_QUESTION_MARK_TOKEN:
        return WASP_TERNARY_EXPR_CONDITIONAL_OP;
        break;
    case WASP_L_BRACKET_TOKEN:
        return WASP_NARY_EXPR_SUBSCRIPT_OP;

    default:
        return WASP_EXPR_UNKNOWN_OP;
    }
}

bool wasp_compiler_parse_expression_with_precedence(wasp_compiler* compiler, uint8_t max_precedence,
                                                    wasp_ast_expr** result)
{
    wasp_ast_expr* expr;

    // Parse value or variable
    // Might contain prefix unary operators
    switch (compiler->lexer.token[0].type)
    {
    // ---- Parentheses ----
    case WASP_L_PARENTHESES_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip left parentheses

        if (!wasp_compiler_parse_expression_with_precedence(compiler, UINT8_MAX, &expr))
            return false;

        if (compiler->lexer.token[0].type != WASP_R_PARENTHESES_TOKEN)
        {
            wasp_compiler_error(compiler, "Expected right parentheses.");
            return false;
        }

        wasp_lexer_advance(&compiler->lexer); // Skip right parentheses

        break;
    }
    case WASP_R_PARENTHESES_TOKEN: // Error handling
    {
        wasp_compiler_error(compiler, "Expected expression before ')'.");

        expr = create_ast_expr(compiler, WASP_EXPR_UNKNOWN);

        return true; // Recovery measures were taken
    }

    // ---- Block ----
    case WASP_L_CURLY_BRACKET_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip left curly bracket

        if (!wasp_compiler_parse_expression_block_with_end_token(compiler, &expr, WASP_R_CURLY_BRACKET_TOKEN))
            return false;

        if (compiler->lexer.token[0].type != WASP_R_CURLY_BRACKET_TOKEN)
        {
            wasp_compiler_error(compiler, "Expected right curly bracket.");
            return false;
        }

        wasp_lexer_advance(&compiler->lexer); // Skip right curly bracket

        break;
    }
    case WASP_R_CURLY_BRACKET_TOKEN: // Error handling
    {
        wasp_compiler_error(compiler, "Expected expression before '}'.");

        expr = create_ast_expr(compiler, WASP_EXPR_UNKNOWN);

        return true; // Recovery measures were taken
    }

    // ---- Unary operator ----
    case WASP_ADDITION_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // +

        wasp_ast_expr* child_node;
        if (!wasp_compiler_parse_expression_with_precedence(compiler, 1, &child_node))
        {
            wasp_compiler_error(compiler, "Expected expression after prefix '+' operator.");

            child_node = create_ast_expr(compiler, WASP_EXPR_UNKNOWN);
        }

        switch (child_node->type)
        {
        case WASP_EXPR_CONST_INTEGER:
        case WASP_EXPR_CONST_NUMBER:
        {
            // Do nothing as the positive unary operator has no effect on these values
            expr = child_node;
            break;
        }
        default:
        {
            // Create node
            expr = create_ast_expr(compiler, WASP_EXPR_OPERATION);
            expr->data.operation.op = WASP_UNARY_EXPR_POSITIVE_OP;
            expr->data.operation.operands.unary.expr_1 = child_node;

            break;
        }
        }

        break;
    }
    case WASP_SUBTRACTION_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // -

        wasp_ast_expr* child_node;
        if (!wasp_compiler_parse_expression_with_precedence(compiler, 1, &child_node))
        {
            wasp_compiler_error(compiler, "Expected expression after prefix '-' operator.");

            child_node = create_ast_expr(compiler, WASP_EXPR_UNKNOWN);
        }

        switch (child_node->type)
        {
        case WASP_EXPR_CONST_INTEGER:
        {
            expr = child_node;
            expr->data.integer_const.value = !expr->data.integer_const.value;

            break;
        }
        case WASP_EXPR_CONST_NUMBER:
        {
            expr = child_node;
            expr->data.number_const.value = !expr->data.number_const.value;

            break;
        }
        default:
        {
            // Create node
            expr = create_ast_expr(compiler, WASP_EXPR_OPERATION);
            expr->data.operation.op = WASP_UNARY_EXPR_NEGATIVE_OP;
            expr->data.operation.operands.unary.expr_1 = child_node;

            break;
        }
        }

        break;
    }
    case WASP_INCREMENT_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip ++

        wasp_ast_expr* child_node;
        if (!wasp_compiler_parse_expression_with_precedence(compiler, 1, &child_node))
        {
            wasp_compiler_error(compiler, "Expected expression after prefix '++' operator.");

            child_node = create_ast_expr(compiler, WASP_EXPR_UNKNOWN);
        }

        // Create node
        expr = create_ast_expr(compiler, WASP_EXPR_OPERATION);
        expr->data.operation.op = WASP_UNARY_EXPR_PRE_INCREMENT_OP;
        expr->data.operation.operands.unary.expr_1 = child_node;

        break;
    }
    case WASP_DECREMENT_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip --

        wasp_ast_expr* child_node;
        if (!wasp_compiler_parse_expression_with_precedence(compiler, 1, &child_node))
        {
            wasp_compiler_error(compiler, "Expected expression after prefix '--' operator.");

            child_node = create_ast_expr(compiler, WASP_EXPR_UNKNOWN);
        }

        // Create node
        expr = create_ast_expr(compiler, WASP_EXPR_OPERATION);
        expr->data.operation.op = WASP_UNARY_EXPR_PRE_INCREMENT_OP;
        expr->data.operation.operands.unary.expr_1 = child_node;

        break;
    }
    case WASP_IDENTIFIER_TOKEN:
    {
        expr = create_ast_expr(compiler, WASP_EXPR_VARIABLE);
        expr->data.variable.name = compiler->lexer.token[0].text;

        wasp_lexer_advance(&compiler->lexer); // Skip identifier

        break;
    }

    // ---- Constant values ----
    case WASP_NULL_KEYWORD_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip null keyword

        expr = create_ast_expr(compiler, WASP_EXPR_CONST_NULL);

        break;
    }
    case WASP_TRUE_KEYWORD_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip true keyword

        expr = create_ast_expr(compiler, WASP_EXPR_CONST_BOOL);
        expr->data.bool_const.value = true;

        break;
    }
    case WASP_FALSE_KEYWORD_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip false keyword

        expr = create_ast_expr(compiler, WASP_EXPR_CONST_BOOL);
        expr->data.bool_const.value = false;

        break;
    }
    case WASP_INTEGER_TOKEN:
    {
        expr = create_ast_expr(compiler, WASP_EXPR_CONST_INTEGER);
        expr->data.integer_const.value = compiler->lexer.token[0].value.integer;

        wasp_lexer_advance(&compiler->lexer); // Skip integer

        break;
    }
    case WASP_NUMBER_TOKEN:
    {
        expr = create_ast_expr(compiler, WASP_EXPR_CONST_NUMBER);
        expr->data.number_const.value = compiler->lexer.token[0].value.number;

        wasp_lexer_advance(&compiler->lexer); // Skip number

        break;
    }
    case WASP_STRING_LITERAL_TOKEN:
    {
        expr = create_ast_expr(compiler, WASP_EXPR_CONST_STRING_LITERAL);
        parse_string_literal(compiler, &expr->data.string_literal_const.value);

        wasp_lexer_advance(&compiler->lexer); // Skip string literal

        break;
    }

    // --- Array ---
    case WASP_L_BRACKET_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip left bracket

        expr = create_ast_expr(compiler, WASP_EXPR_ARRAY);

        wasp_ast_expr_list_item* last_item = NULL;

        while (compiler->lexer.token[0].type != WASP_R_BRACKET_TOKEN)
        {
            wasp_ast_expr* child_expr;

            // Parse value
            if (!wasp_compiler_parse_expression_with_precedence(compiler, UINT8_MAX, &child_expr))
            {
                wasp_compiler_error(compiler, "Expected expression as array item.");
                return false;
            }

            // Add array item
            {
                wasp_ast_expr_list_item* new_item = create_ast_expr_list_item(compiler, child_expr);

                if (last_item != NULL)
                    last_item->next_item = new_item;
                else
                    expr->data.array.first_item = new_item;

                last_item = new_item;
            }

            // Parse comma (or end bracket)
            {
                if (compiler->lexer.token[0].type == WASP_COMMA_TOKEN)
                    wasp_lexer_advance(&compiler->lexer); // Skip comma
                else if (compiler->lexer.token[0].type != WASP_R_BRACKET_TOKEN)
                {
                    wasp_compiler_error(compiler, "Expected ',' or ']' after array item.");

                    // TODO Handle error
                    return false;
                }
            }
        }

        if (last_item != NULL)
            last_item->next_item = NULL; // End linked list

        wasp_lexer_advance(&compiler->lexer); // Skip right bracket

        break;
    }
    case WASP_R_BRACKET_TOKEN: // Error handling
    {
        wasp_compiler_error(compiler, "Expected expression before ']'.");

        expr = create_ast_expr(compiler, WASP_EXPR_UNKNOWN);

        return true; // Recovery measures were taken
    }

    // ---- If Else ----
    case WASP_IF_KEYWORD_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip if keyword

        wasp_ast_expr* condition_expr;

        // Parse condition expression
        if (!wasp_compiler_parse_expression_with_precedence(compiler, UINT8_MAX, &condition_expr))
            return false;

        wasp_ast_expr* then_child_expr;
        wasp_ast_expr* else_child_expr;

        // Parse then expression
        if (!wasp_compiler_parse_expression_with_precedence(compiler, UINT8_MAX, &then_child_expr))
            then_child_expr = NULL;

        // Parse else expression if present
        if (compiler->lexer.token[0].type == WASP_ELSE_KEYWORD_TOKEN)
        {
            wasp_lexer_advance(&compiler->lexer); // Skip else keyword

            if (!wasp_compiler_parse_expression_with_precedence(compiler, UINT8_MAX, &else_child_expr))
                else_child_expr = NULL;
        }
        else
            else_child_expr = NULL;

        expr = create_ast_expr(compiler, WASP_EXPR_IF_ELSE);
        expr->data.if_else.condition_expr = condition_expr;
        expr->data.if_else.optional_then_expr = then_child_expr;
        expr->data.if_else.optional_else_expr = else_child_expr;

        break;
    }

    default:
        return false;
    }

    // Parse post operator
    switch (compiler->lexer.token[0].type)
    {
    case WASP_INCREMENT_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip ++

        wasp_ast_expr* unary_expr_node = create_ast_expr(compiler, WASP_EXPR_OPERATION);
        unary_expr_node->data.operation.op = WASP_UNARY_EXPR_POST_INCREMENT_OP;
        unary_expr_node->data.operation.operands.unary.expr_1 = expr;

        expr = unary_expr_node;
        break;
    }
    case WASP_DECREMENT_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip --

        wasp_ast_expr* unary_expr_node = create_ast_expr(compiler, WASP_EXPR_OPERATION);
        unary_expr_node->data.operation.op = WASP_UNARY_EXPR_POST_DECREMENT_OP;
        unary_expr_node->data.operation.operands.unary.expr_1 = expr;

        expr = unary_expr_node;
        break;
    }
    default:
        break;
    }

    // Parse binary, ternary and nary operators
    wasp_ast_expr_operator op = get_operator(compiler->lexer.token[0]);
    while (op != WASP_EXPR_UNKNOWN_OP)
    {
        uint8_t op_precedence = OP_PRECEDENCE[op];
        if (op_precedence > max_precedence)
            break;

        // Create node
        wasp_ast_expr* expr_node = create_ast_expr(compiler, WASP_EXPR_OPERATION);
        expr_node->data.operation.op = op;
        expr_node->data.operation.operands.unary.expr_1 = expr;

        if (wasp_is_expr_operator_binary(op))
        {
            wasp_lexer_advance(&compiler->lexer); // Skip operator

            if (!wasp_compiler_parse_expression_with_precedence(compiler, op_precedence - 1,
                                                                &expr_node->data.operation.operands.binary.expr_2))
            {
                wasp_compiler_error(compiler, "Expected expression after binary operator.");

                expr_node->data.operation.operands.binary.expr_2 = create_ast_expr(compiler, WASP_EXPR_UNKNOWN);
            }
        }
        else
        {
            switch (op)
            {
            case WASP_TERNARY_EXPR_CONDITIONAL_OP:
            {
                wasp_lexer_advance(&compiler->lexer); // Skip ?

                // Parse second operand
                if (!wasp_compiler_parse_expression_with_precedence(compiler, UINT8_MAX,
                                                                    &expr_node->data.operation.operands.ternary.expr_2))
                {
                    wasp_compiler_error(compiler, "Expected expression after '?' of a conditional operator.");

                    expr_node->data.operation.operands.ternary.expr_2 = create_ast_expr(compiler, WASP_EXPR_UNKNOWN);
                }

                // Parse colon
                {
                    if (compiler->lexer.token[0].type == WASP_COLON_TOKEN)
                        wasp_lexer_advance(&compiler->lexer); // Skip colon
                    else
                        wasp_compiler_error(compiler, "Expected ':' after second operand of a conditional operator.");
                }

                // Parse third operand
                if (!wasp_compiler_parse_expression_with_precedence(compiler, op_precedence - 1,
                                                                    &expr_node->data.operation.operands.ternary.expr_3))
                {
                    wasp_compiler_error(compiler, "Expected expression after ':' of a conditional operator.");

                    expr_node->data.operation.operands.ternary.expr_3 = create_ast_expr(compiler, WASP_EXPR_UNKNOWN);
                }

                break;
            }
            case WASP_NARY_EXPR_SUBSCRIPT_OP:
            {
                wasp_lexer_advance(&compiler->lexer); // Skip left bracket

                // Add subject node
                wasp_ast_expr_list_item* last_item = create_ast_expr_list_item(compiler, expr);
                expr_node->data.operation.operands.nary.first_item = last_item;

                while (compiler->lexer.token[0].type != WASP_R_BRACKET_TOKEN)
                {
                    wasp_ast_expr* value_expr_node;

                    // Parse value
                    if (!wasp_compiler_parse_expression_with_precedence(compiler, UINT8_MAX, &value_expr_node))
                    {
                        wasp_compiler_error(compiler, "Expected expression as subscript operator index.");
                        return false;
                    }

                    // Add array item
                    {
                        wasp_ast_expr_list_item* new_item = create_ast_expr_list_item(compiler, value_expr_node);

                        last_item->next_item = new_item;
                        last_item = new_item;
                    }

                    // Parse comma (or end bracket)
                    {
                        if (compiler->lexer.token[0].type == WASP_COMMA_TOKEN)
                            wasp_lexer_advance(&compiler->lexer); // Skip comma
                        else if (compiler->lexer.token[0].type != WASP_R_BRACKET_TOKEN)
                        {
                            wasp_compiler_error(compiler, "Expected ',' or ']' after subscript index.");

                            // TODO Handle error
                            return false;
                        }
                    }
                }

                if (last_item != NULL)
                    last_item->next_item = NULL; // End linked list

                wasp_lexer_advance(&compiler->lexer); // Skip right bracket
            }
            default:
                break;
            }
        }

        expr = expr_node;

        // Read next operator
        op = get_operator(compiler->lexer.token[0]);
    }

    *result = expr;
    return true;
}

bool wasp_compiler_parse_expression(wasp_compiler* compiler, wasp_ast_expr** result)
{
    return wasp_compiler_parse_expression_with_precedence(compiler, UINT8_MAX, result);
}

// -------- Statement Parsing --------
bool wasp_compiler_parse_statement(wasp_compiler* compiler, wasp_ast_expr** result, bool* out_requires_semicolon,
                                   bool* out_has_semicolon)
{
    assert(result != NULL);
    assert(out_requires_semicolon != NULL);
    assert(out_has_semicolon != NULL);

    wasp_ast_expr* expr;

    bool requires_semicolon = true;

    switch (compiler->lexer.token[0].type)
    {
    // ---- While Loop ----
    case WASP_WHILE_KEYWORD_TOKEN:
    {
        requires_semicolon = false; // A semicolon is not required after a while statement

        wasp_lexer_advance(&compiler->lexer); // Skip while keyword

        wasp_ast_expr* condition_expr;

        // Parse condition expression
        if (!wasp_compiler_parse_expression_with_precedence(compiler, UINT8_MAX, &condition_expr))
            return false;

        wasp_ast_expr* child_expr;
        bool requires_semicolon, has_semicolon;

        // Parse child expression
        if (!wasp_compiler_parse_statement(compiler, &child_expr, &requires_semicolon, &has_semicolon))
            child_expr = NULL;

        expr = create_ast_expr(compiler, WASP_EXPR_IF_ELSE);
        expr->data.while_loop.condition_expr = condition_expr;
        expr->data.while_loop.body_expr = child_expr;

        break;
    }

    // ---- Statement ----
    case WASP_BREAK_KEYWORD_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip return keyword

        break;
    }
    case WASP_CONTINUE_KEYWORD_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip return keyword

        break;
    }
    case WASP_RETURN_KEYWORD_TOKEN:
    {
        wasp_lexer_advance(&compiler->lexer); // Skip return keyword

        wasp_ast_expr* child_expr;
        if (!wasp_compiler_parse_expression_with_precedence(compiler, UINT8_MAX, &child_expr))
            child_expr = NULL; // As return has an optional value this is not treated as an error

        // Create node
        expr = create_ast_expr(compiler, WASP_EXPR_RETURN);
        expr->data.ret.optional_value = child_expr;

        break;
    }

    case WASP_IF_KEYWORD_TOKEN:
        requires_semicolon = false;
    default:
        if (!wasp_compiler_parse_expression(compiler, &expr))
            return false;
    }

    *out_requires_semicolon = requires_semicolon;
    if (compiler->lexer.token[0].type == WASP_SEMICOLON_TOKEN)
    {
        if (!requires_semicolon)
            wasp_compiler_warning(compiler, "A semicolon is not required here.");

        wasp_lexer_advance(&compiler->lexer); // Skip semicolon

        *out_has_semicolon = true;
    }
    else
        *out_has_semicolon = false;
}

bool wasp_compiler_parse_statement_block_with_end_token(wasp_compiler* compiler, wasp_ast_expr** result,
                                                        wasp_token_type end_token)
{
    assert(result != NULL);

    wasp_ast_expr* expr = create_ast_expr(compiler, WASP_EXPR_BLOCK);

    wasp_ast_expr_list_item* last_item = NULL;

    bool last_expression_passed_back = false;
    while (compiler->lexer.token[0].type != end_token)
    {
        if (!last_expression_passed_back)
            wasp_compiler_error(compiler, "Expected a semicolon as only the last statement can pass back.");

        wasp_ast_expr* child_expr;
        bool requires_semicolon, has_semicolon;

        // Parse value
        if (!wasp_compiler_parse_statement(compiler, &child_expr, &requires_semicolon, &has_semicolon))
        {
            wasp_compiler_error(compiler, "Expected statement inside block.");
            return false;
        }

        // Add block item
        {
            wasp_ast_expr_list_item* new_item = create_ast_expr_list_item(compiler, child_expr);

            if (last_item != NULL)
                last_item->next_item = new_item;
            else
                expr->data.block.first_item = new_item;

            last_item = new_item;
        }

        // Parse semicolon, end bracket or eof
        {
            if (!has_semicolon)
            {
                // Raise pass back flag
                last_expression_passed_back = true;

                if (compiler->lexer.token[0].type != end_token && !requires_semicolon)
                {
                    wasp_compiler_error(compiler, "Expected ';' or pass back after statement.");

                    // TODO Handle error
                    return false;
                }
            }
        }
    }

    expr->data.block.has_pass_back = last_expression_passed_back;

    *result = expr;
    return true;
}

bool wasp_compiler_parse_statement_block(wasp_compiler* compiler, wasp_ast_expr** result)
{
    return wasp_compiler_parse_statement_block_with_end_token(compiler, result, WASP_EOF_TOKEN);
}