#pragma once
#include "wasp_ast.h"
#include "wasp_common.h"
#include "wasp_lazy_allocator.h"
#include "wasp_lexer.h"

typedef struct wasp_compiler
{
    wasp_logger logger;
    wasp_lexer lexer;

    wasp_lallocator allocator;

    jmp_buf allocation_jmp_buf;

    size_t error_count;
} wasp_compiler;

bool wasp_create_compiler(wasp_compiler* compiler, unsigned char* text, size_t text_length);

bool wasp_compiler_set_allocation_jmp(wasp_compiler* compiler);

void wasp_compiler_allocation_error(wasp_compiler* compiler, const char* type);
void wasp_compiler_error(wasp_compiler* compiler, const char* format, ...);
void wasp_compiler_warning(wasp_compiler* compiler, const char* format, ...);

bool wasp_compiler_parse_type(wasp_compiler* compiler, wasp_ast_type* result);

bool wasp_compiler_parse_decl(wasp_compiler* compiler, wasp_ast_decl* result);

bool wasp_compiler_parse_expression(wasp_compiler* compiler, wasp_ast_expr** result);

bool wasp_compiler_parse(wasp_compiler* compiler, wasp_ast_expr);