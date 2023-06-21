#include "wasp_ast_expr.h"

bool wasp_is_expr_operator_unary(wasp_ast_expr_operator op)
{
    return op >= WASP_UNARY_EXPR_REFERENCE_OP && op <= WASP_UNARY_EXPR_BITWISE_NOT_OP;
}
bool wasp_is_expr_operator_binary(wasp_ast_expr_operator op)
{
    return op >= WASP_BINARY_EXPR_MEMBER_ACCESS_OP && op <= WASP_BINARY_EXPR_SUBTRACTION_ASSIGNMENT_OP;
}
bool wasp_is_expr_operator_ternary(wasp_ast_expr_operator op)
{
    return op == WASP_TERNARY_EXPR_CONDITIONAL_OP;
}
bool wasp_is_expr_operator_nary(wasp_ast_expr_operator op)
{
    return op == WASP_NARY_EXPR_SUBSCRIPT_OP;
}

static const char* wasp_expr_names[] = {
    "unknown expr",    "operation expr",     "variable expr",     "null const expr",
    "bool const expr", "integer const expr", "number const expr", "string literal const expr",
    "array expr",      "map expr",
};

static const char* wasp_expr_op_names[] = {
    "unknown op",

    // Unary
    "reference op",
    "positive op",
    "negative op",
    "pre increment op",
    "post increment op",
    "pre decrement op",
    "post decrement op",
    "logical not op",
    "bitwise not op",

    // Binary
    "member access op",

    "bitwise and op",
    "bitwise xor op",
    "bitwise or op",
    "bitwise left shift op",
    "bitwise right shift op",

    "multiplication op",
    "division op",
    "modulus op",
    "addition op",
    "subtraction op",

    "greater op",
    "greater equal op",
    "lesser op",
    "lesser equal op",

    "equal op",
    "not equal op",

    "logical and op",
    "logical xor op",
    "logical or op",

    "assignment op",

    "multiplication assignment op",
    "division assignment op",
    "modulus assignment op",
    "addition assignment op",
    "subtraction assignment op",

    "logical and assignment op",
    "logical xor assignment op",
    "logical or assignment op",

    "bitwise and assignment op",
    "bitwise xor assignment op",
    "bitwise or assignment op",
    "bitwise left shift assignment op",
    "bitwise right shift assignment op",

    // Ternary
    "conditional op",
    "slicing op",

    // Nary
    "subscript op",
};

void wasp_ast_print_expr(wasp_ast_expr* expr, size_t depth)
{
    for (size_t i = 0; i < depth; i++)
        printf("   ");
    depth++;

    if (expr != NULL)
    {
        switch (expr->type)
        {
        case WASP_EXPR_UNKNOWN:
            printf("|- unknown expression\n");
            break;
        case WASP_EXPR_OPERATION:
            if (wasp_is_expr_operator_unary(expr->data.operation.op))
            {
                printf("|- unary expression: %s\n", wasp_expr_op_names[expr->data.operation.op]);
                wasp_ast_print_expr(expr->data.operation.operands.unary.expr_1, depth);
            }
            else if (wasp_is_expr_operator_binary(expr->data.operation.op))
            {
                printf("|- binary expression: %s\n", wasp_expr_op_names[expr->data.operation.op]);
                wasp_ast_print_expr(expr->data.operation.operands.binary.expr_1, depth);
                wasp_ast_print_expr(expr->data.operation.operands.binary.expr_2, depth);
            }
            else if (wasp_is_expr_operator_ternary(expr->data.operation.op))
            {
                printf("|- ternary expression: %s\n", wasp_expr_op_names[expr->data.operation.op]);
                wasp_ast_print_expr(expr->data.operation.operands.ternary.expr_1, depth);
                wasp_ast_print_expr(expr->data.operation.operands.ternary.expr_2, depth);
                wasp_ast_print_expr(expr->data.operation.operands.ternary.expr_3, depth);
            }
            else if (wasp_is_expr_operator_nary(expr->data.operation.op))
            {
                printf("|- nary expression: %s\n", wasp_expr_op_names[expr->data.operation.op]);

                for (wasp_ast_expr_list_item* item = expr->data.operation.operands.nary.first_item; item != NULL;
                     item = item->next_item)
                {
                    wasp_ast_print_expr(item->expr, depth);
                }
            }
            break;
        case WASP_EXPR_VARIABLE:
            printf("|- variable expression: %.*s\n", (int)expr->data.variable.name.data_length,
                   expr->data.variable.name.data);
            break;
        case WASP_EXPR_CONST_NULL:
            printf("|- null value expression\n");
            break;
        case WASP_EXPR_CONST_BOOL:
            printf("|- bool value expression: %s\n", expr->data.bool_const.value ? "true" : "false");
            break;
        case WASP_EXPR_CONST_INTEGER:
            printf("|- integer value expression: %zu\n", expr->data.integer_const.value);
            break;
        case WASP_EXPR_CONST_NUMBER:
            printf("|- number value expression: %f\n", expr->data.number_const.value);
            break;
        case WASP_EXPR_CONST_STRING_LITERAL:
            printf("|- string value expression: %.*s\n", (int)expr->data.string_literal_const.value.data_length,
                   expr->data.string_literal_const.value.data);
            break;
        case WASP_EXPR_ARRAY:
            printf("|- array value expression\n");

            for (wasp_ast_expr_list_item* item = expr->data.array.first_item; item != NULL; item = item->next_item)
                wasp_ast_print_expr(item->expr, depth);
            break;
        case WASP_EXPR_BLOCK:
            if (expr->data.block.has_pass_back)
                printf("|- block expression with pass back\n");
            else
                printf("|- block expression\n");

            for (wasp_ast_expr_list_item* item = expr->data.block.first_item; item != NULL; item = item->next_item)
                wasp_ast_print_expr(item->expr, depth);
            break;
        case WASP_EXPR_IF_ELSE:
            if (expr->data.if_else.optional_else_expr == NULL)
            {
                printf("|- if expression\n");
                wasp_ast_print_expr(expr->data.if_else.condition_expr, depth);

                if (expr->data.if_else.optional_then_expr != NULL)
                {
                    putchar('\n');
                    wasp_ast_print_expr(expr->data.if_else.optional_then_expr, depth);
                }
            }
            else
            {
                printf("|- if else expression\n");
                wasp_ast_print_expr(expr->data.if_else.condition_expr, depth);

                // Then expr
                if (expr->data.if_else.optional_then_expr != NULL)
                {
                    putchar('\n');
                    wasp_ast_print_expr(expr->data.if_else.optional_then_expr, depth);
                }

                // Else expr
                {
                    putchar('\n');
                    wasp_ast_print_expr(expr->data.if_else.optional_else_expr, depth);
                }
            }
            break;
        case WASP_EXPR_BREAK:
            printf("|- break statement\n");
            break;
        case WASP_EXPR_CONTINUE:
            printf("|- continue statement\n");
            break;
        case WASP_EXPR_RETURN:
            printf("|- return statement\n");

            if (expr->data.ret.optional_value != NULL)
                wasp_ast_print_expr(expr->data.ret.optional_value, depth);
            break;
        }
    }
    else
        printf("|- null node\n");
}