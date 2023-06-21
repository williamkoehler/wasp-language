#pragma once
#include "wasp_ast.h"
#include "wasp_ast_type.h"
#include "wasp_common.h"

typedef enum wasp_ast_expr_type : uint8_t
{
    WASP_EXPR_UNKNOWN,
    WASP_EXPR_OPERATION,
    WASP_EXPR_VARIABLE,
    WASP_EXPR_CONST_NULL,
    WASP_EXPR_CONST_BOOL,
    WASP_EXPR_CONST_INTEGER,
    WASP_EXPR_CONST_NUMBER,
    WASP_EXPR_CONST_STRING_LITERAL,
    WASP_EXPR_ARRAY,

    WASP_EXPR_BLOCK,
    WASP_EXPR_IF_ELSE,
    WASP_EXPR_WHILE_LOOP,
    WASP_EXPR_DO_WHILE_LOOP,
    WASP_EXPR_FOR_LOOP,
    WASP_EXPR_BREAK,
    WASP_EXPR_CONTINUE,
    WASP_EXPR_RETURN,

    WASP_EXPR_STRUCT_DECL,
    WASP_EXPR_FUNCTION_DECL,
    WASP_EXPR_LAMBDA_FUNCTION_DECL,
} wasp_ast_expr_type;

typedef enum wasp_ast_expr_operator : uint8_t
{
    WASP_EXPR_UNKNOWN_OP,

    // Unary
    WASP_UNARY_EXPR_REFERENCE_OP,

    WASP_UNARY_EXPR_POSITIVE_OP,
    WASP_UNARY_EXPR_NEGATIVE_OP,

    WASP_UNARY_EXPR_PRE_INCREMENT_OP,
    WASP_UNARY_EXPR_POST_INCREMENT_OP,
    WASP_UNARY_EXPR_PRE_DECREMENT_OP,
    WASP_UNARY_EXPR_POST_DECREMENT_OP,

    WASP_UNARY_EXPR_LOGICAL_NOT_OP,

    WASP_UNARY_EXPR_BITWISE_NOT_OP,

    // Binary
    WASP_BINARY_EXPR_MEMBER_ACCESS_OP,

    WASP_BINARY_EXPR_BITWISE_AND_OP,
    WASP_BINARY_EXPR_BITWISE_XOR_OP,
    WASP_BINARY_EXPR_BITWISE_OR_OP,
    WASP_BINARY_EXPR_BITWISE_LEFT_SHIFT_OP,
    WASP_BINARY_EXPR_BITWISE_RIGHT_SHIFT_OP,

    WASP_BINARY_EXPR_MULTIPLICATION_OP,
    WASP_BINARY_EXPR_DIVISION_OP,
    WASP_BINARY_EXPR_MODULUS_OP,
    WASP_BINARY_EXPR_ADDITION_OP,
    WASP_BINARY_EXPR_SUBTRACTION_OP,

    WASP_BINARY_EXPR_GREATER_OP,
    WASP_BINARY_EXPR_GREATER_EQUAL_OP,
    WASP_BINARY_EXPR_LESSER_OP,
    WASP_BINARY_EXPR_LESSER_EQUAL_OP,

    WASP_BINARY_EXPR_EQUAL_OP,
    WASP_BINARY_EXPR_NOT_EQUAL_OP,

    WASP_BINARY_EXPR_LOGICAL_AND_OP,
    WASP_BINARY_EXPR_LOGICAL_XOR_OP,
    WASP_BINARY_EXPR_LOGICAL_OR_OP,

    WASP_BINARY_ASSIGNMENT_OP,

    WASP_BINARY_EXPR_MULTIPLICATION_ASSIGNMENT_OP,
    WASP_BINARY_EXPR_DIVISION_ASSIGNMENT_OP,
    WASP_BINARY_EXPR_MODULUS_ASSIGNMENT_OP,
    WASP_BINARY_EXPR_ADDITION_ASSIGNMENT_OP,
    WASP_BINARY_EXPR_SUBTRACTION_ASSIGNMENT_OP,

    // Ternary
    WASP_TERNARY_EXPR_CONDITIONAL_OP,

    // Nary
    WASP_NARY_EXPR_SUBSCRIPT_OP,
} wasp_ast_expr_operator;

bool wasp_is_expr_operator_unary(wasp_ast_expr_operator op);
bool wasp_is_expr_operator_binary(wasp_ast_expr_operator op);
bool wasp_is_expr_operator_ternary(wasp_ast_expr_operator op);
bool wasp_is_expr_operator_nary(wasp_ast_expr_operator op);

struct wasp_ast_expr
{
    wasp_ast_expr_type type;

    union
    {
        struct
        {
            wasp_ast_expr_operator op;

            union
            {
                struct
                {
                    wasp_ast_expr* expr_1;
                } unary;
                struct
                {
                    wasp_ast_expr* expr_1;
                    wasp_ast_expr* expr_2;
                } binary;
                struct
                {
                    wasp_ast_expr* expr_1;
                    wasp_ast_expr* expr_2;
                    wasp_ast_expr* expr_3;
                } ternary;
                struct
                {
                    wasp_ast_expr_list_item* first_item;
                } nary;
            } operands;
        } operation;
        struct
        {
            wasp_string name;
        } variable;
        struct
        {
            bool value;
        } bool_const;
        struct
        {
            int64_t value;
        } integer_const;
        struct
        {
            double value;
        } number_const;
        struct
        {
            wasp_string value;
        } string_literal_const;
        struct
        {
            wasp_ast_expr_list_item* first_item;
        } array;

        struct
        {
            wasp_ast_expr_list_item* first_item;
            bool has_pass_back;
        } block;

        struct
        {
            wasp_ast_expr* condition_expr;
            wasp_ast_expr* optional_then_expr;
            wasp_ast_expr* optional_else_expr;
        } if_else;

        struct
        {
            wasp_ast_expr* condition_expr;
            wasp_ast_expr* body_expr;
        } while_loop;

        struct
        {
            wasp_ast_expr* optional_init_expr;
            wasp_ast_expr* optional_condition_expr;
            wasp_ast_expr* optional_increment_expr;
            wasp_ast_expr* body_expr;
        } for_loop;

        struct
        {
            wasp_ast_expr* optional_value;
        } ret;

        struct
        {
            wasp_string name;
            struct
            {
                wasp_ast_decl_list_item* first_item;
            } declarations;
        } struct_decl;

        struct
        {
            wasp_string name;
            struct
            {
                wasp_ast_decl_list_item* first_item;
            } declarations;
            wasp_ast_type return_type;
        } function_decl;

        struct
        {
            struct
            {
                wasp_ast_decl_list_item* first_item;
            } declarations;
            wasp_ast_type return_type;
        } lambda_function_decl;
    } data;
};

struct wasp_ast_expr_list_item
{
    wasp_ast_expr* expr;
    wasp_ast_expr_list_item* next_item;
};

void wasp_ast_print_expr(wasp_ast_expr* expr, size_t depth);