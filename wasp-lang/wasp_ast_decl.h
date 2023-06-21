#pragma once
#include "wasp_ast.h"
#include "wasp_ast_type.h"
#include "wasp_common.h"

struct wasp_ast_decl
{
    wasp_ast_type type;
    wasp_string name;
};

struct wasp_ast_decl_list_item
{
    wasp_ast_decl decl;
    wasp_ast_decl_list_item* next_item;
};