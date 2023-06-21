#pragma once
#include "wasp_common.h"
#include "wasp_ast.h"

struct wasp_ast_type
{
    wasp_string name;

    bool is_reference;

    struct
    {
        wasp_ast_type_list_item* first_item;
    } generics;
};

struct wasp_ast_type_list_item
{
    wasp_ast_type type;
    wasp_ast_type_list_item* next_item;
};

void wasp_ast_print_type(wasp_ast_type type, size_t depth);